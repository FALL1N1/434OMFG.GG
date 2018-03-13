/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: Zulaman
SD%Complete: 90
SDComment: Forest Frog will turn into different NPC's. Workaround to prevent new entry from running this script
SDCategory: Zul'Aman
EndScriptData */

/* ContentData
npc_forest_frog
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "zulaman.h"
#include "Player.h"
#include "SpellInfo.h"
#include "SpellScript.h"

/*######
## npc_forest_frog
######*/

#define SPELL_REMOVE_AMANI_CURSE    43732
#define SPELL_PUSH_MOJO             43923
#define ENTRY_FOREST_FROG           24396

class npc_forest_frog : public CreatureScript
{
    public:

        npc_forest_frog()
            : CreatureScript("npc_forest_frog")
        {
        }

        struct npc_forest_frogAI : public ScriptedAI
        {
            npc_forest_frogAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
            }

            InstanceScript* instance;

            void Reset()
            {
                me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_VENDOR);
            }

            void EnterCombat(Unit* /*who*/) {}

            void DoSpawnRandom()
            {
                if (instance)
                {
                    uint32 cEntry = 0;
                    switch (rand()%10)
                    {
                        case 0: cEntry = 24397; break;          //Mannuth
                        case 1: cEntry = 24403; break;          //Deez
                        case 2: cEntry = 24404; break;          //Galathryn
                        case 3: cEntry = 24405; break;          //Adarrah
                        case 4: cEntry = 24406; break;          //Fudgerick
                        case 5: cEntry = 24407; break;          //Darwen
                        case 6: cEntry = 24445; break;          //Mitzi
                        case 7: cEntry = 24448; break;          //Christian
                        case 8: cEntry = 24453; break;          //Brennan
                        case 9: cEntry = 24455; break;          //Hollee
                    }

                    if (!instance->GetData(TYPE_RAND_VENDOR_1))
                        if (rand()%10 == 1) cEntry = 24408;      //Gunter
                    if (!instance->GetData(TYPE_RAND_VENDOR_2))
                        if (rand()%10 == 1) cEntry = 24409;      //Kyren

                    if (cEntry)
                        me->UpdateEntry(cEntry);

                    if (cEntry == 24408)
                    {
                        instance->SetData(TYPE_RAND_VENDOR_1, DONE);
                        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_VENDOR);
                    }
                    else if (cEntry == 24409)
                    {
                        instance->SetData(TYPE_RAND_VENDOR_2, DONE);
                        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_VENDOR);
                    }
                }
            }

            void SpellHit(Unit* caster, const SpellInfo* spell)
            {
                if (spell->Id == SPELL_REMOVE_AMANI_CURSE && caster->GetTypeId() == TYPEID_PLAYER && me->GetEntry() == ENTRY_FOREST_FROG)
                {
                    //increase or decrease chance of mojo?
                    if (rand()%99 == 50) DoCast(caster, SPELL_PUSH_MOJO, true);
                    else DoSpawnRandom();
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_forest_frogAI(creature);
        }
};

/*######
## npc_zulaman_hostage
######*/

#define GOSSIP_HOSTAGE1        "Please sace me!"

class npc_zulaman_hostage : public CreatureScript
{
    public:
        npc_zulaman_hostage() : CreatureScript("npc_zulaman_hostage") { }

        struct npc_zulaman_hostageAI : public ScriptedAI
        {
            npc_zulaman_hostageAI(Creature* creature) : ScriptedAI(creature)
            {
            }

            void Reset() {}

            void EnterCombat(Unit* /*who*/) {}

            void JustDied(Unit* /*killer*/)
            {
            }

            void UpdateAI(uint32 const /*diff*/)
            {
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_zulaman_hostageAI(creature);
        }

        bool OnGossipHello(Player* player, Creature* creature)
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HOSTAGE1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
            return true;
        }

        bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
        {
            player->PlayerTalkClass->ClearMenus();

            if (action == GOSSIP_ACTION_INFO_DEF + 1)
                player->CLOSE_GOSSIP_MENU();

            if (!creature->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP))
                return true;

            creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

            if (InstanceScript* instance = creature->GetInstanceScript())
            {
                //uint8 progress = instance->GetData(DATA_CHESTLOOTED);
                Map::PlayerList const& players = creature->GetMap()->GetPlayers();
                for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                    if (Player* player = itr->getSource())
                        player->KilledMonsterCredit(creature->GetEntry());

                instance->SetData(DATA_CHESTLOOTED, 0);
                float x, y, z;
                creature->GetPosition(x, y, z);
                uint32 entry = creature->GetEntry();
            }
            return true;
        }
};

/*######
## npc_harrison_jones
######*/

enum Says
{
    SAY_HARRISON_0                    = 0,
    SAY_HARRISON_1                    = 1,
    SAY_HARRISON_2                    = 0,
    SAY_HARRISON_3                    = 1
};

enum Spells
{
    SPELL_BANGING_THE_GONG            = 45225,
    SPELL_STEALTH                     = 34189,
    SPELL_COSMETIC_SPEAR_THROW        = 43647
};

enum Events
{
    GONG_EVENT_1                      = 1,
    GONG_EVENT_2                      = 2,
    GONG_EVENT_3                      = 3,
    GONG_EVENT_4                      = 4,
    GONG_EVENT_5                      = 5,
    GONG_EVENT_6                      = 6,
    GONG_EVENT_7                      = 7,
    GONG_EVENT_8                      = 8,
    GONG_EVENT_9                      = 9,
    GONG_EVENT_10                     = 10,
    GONG_EVENT_11                     = 11,
    DATA_HIT_GONG,
};

enum Waypoints
{
    HARRISON_MOVE_1                   = 860440,
    HARRISON_MOVE_2                   = 860441,
    HARRISON_MOVE_3                   = 860442
};

enum DisplayIds
{
    MODEL_HARRISON_JONES_0              = 22340,
    MODEL_HARRISON_JONES_1              = 22354,
    MODEL_HARRISON_JONES_2              = 22347
};

enum EntryIds
{
    NPC_HARRISON_JONES_1                = 24375,
    NPC_HARRISON_JONES_2                = 24365,
    NPC_AMANISHI_GUARDIAN               = 23597,
};

enum Weapons
{
    WEAPON_MACE                         = 5301,
    WEAPON_SPEAR                        = 13631
};

class npc_harrison_jones : public CreatureScript
{
    public:

        npc_harrison_jones()
            : CreatureScript("npc_harrison_jones")
        {
        }

        struct npc_harrison_jonesAI : public ScriptedAI
        {
            npc_harrison_jonesAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
            }

            InstanceScript* instance;

            uint8 _gongEvent;
            uint32 _gongTimer;
            uint32 gongHitCount;

            void Reset()
            {
                _gongEvent = 0;
                _gongTimer = 0;
                gongHitCount = 0;
            }

            void EnterCombat(Unit* /*who*/) {}

            void sGossipSelect(Player* player, uint32 sender, uint32 action)
            {
                if (!instance->IsDone(DATA_GONGEVENT))
                {
                    if (me->GetCreatureTemplate()->GossipMenuId == sender && !action)
                    {
                        player->CLOSE_GOSSIP_MENU();
                        me->SetFacingToObject(player);
                        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        Talk(SAY_HARRISON_0);
                        _gongEvent = GONG_EVENT_1;
                        _gongTimer = 4000;
                    }
                }
                else
                {
                    if (GameObject* gate = me->GetMap()->GetGameObject(instance->GetData64(GO_MASSIVE_GATE)))
                        gate->SetGoState(GO_STATE_ACTIVE);
                    me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                }
            }

            void SpellHit(Unit*, const SpellInfo* spell)
            {
                if (spell->Id == SPELL_COSMETIC_SPEAR_THROW)
                {
                    me->RemoveAllAuras();
                    me->SetEntry(NPC_HARRISON_JONES_2);
                    me->SetDisplayId(MODEL_HARRISON_JONES_2);
                    me->SetTarget(0);
                    me->SetByteValue(UNIT_FIELD_BYTES_1, 0, UNIT_STAND_STATE_DEAD);
                    me->SetFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
                    if (instance)
                        instance->SetBossState(DATA_GONGEVENT, DONE_HM);
                }
            }

            void SetData(uint32 id, uint32 value)
            {
                if (id == DATA_HIT_GONG)
                    gongHitCount += value;
                if (gongHitCount == 5)
                {
                    instance->SetBossState(DATA_GONGEVENT, IN_PROGRESS);
                    if (_gongEvent == GONG_EVENT_4)
                        _gongTimer = 1000;
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (_gongEvent)
                {
                    if (_gongTimer <= diff)
                    {
                        switch (_gongEvent)
                        {
                            case GONG_EVENT_1:
                                // me->GetMotionMaster()->MovePath(HARRISON_MOVE_1, false);
                                me->GetMotionMaster()->MovePoint(0, 132.16f, 1642.87f, 42.02f, false);
                                _gongEvent = GONG_EVENT_2;
                                _gongTimer = 12000;
                                break;
                            case GONG_EVENT_2:
                                me->SetFacingTo(6.235659f);
                                Talk(SAY_HARRISON_1);
                                DoCast(me, SPELL_BANGING_THE_GONG);
                                me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 0, uint32(WEAPON_MACE));
                                me->SetSheath(SHEATH_STATE_MELEE);
                                _gongEvent = GONG_EVENT_3;
                                _gongTimer = 4000;
                                break;
                            case GONG_EVENT_3:
                                if (GameObject* gong = me->GetMap()->GetGameObject(instance->GetData64(GO_STRANGE_GONG)))
                                    gong->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                                _gongEvent = GONG_EVENT_4;
                                instance->SetBossState(DATA_GONGEVENT, IN_PROGRESS);
                                _gongTimer = 5000;
                                //                                _gongTimer = 105000;
                                break;
                            case GONG_EVENT_4:
                                me->RemoveAura(SPELL_BANGING_THE_GONG);
                                if (GameObject* gong = me->GetMap()->GetGameObject(instance->GetData64(GO_STRANGE_GONG)))
                                    gong->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);
                                if (instance->GetBossState(DATA_GONGEVENT) == IN_PROGRESS)
                                {
                                    me->GetMotionMaster()->MovePath(HARRISON_MOVE_2, false);
                                    _gongEvent = GONG_EVENT_5;
                                    _gongTimer = 5000;
                                }
                                else
                                {
                                    _gongTimer = 1000;
                                    _gongEvent = GONG_EVENT_9;
                                }
                                break;
                            case GONG_EVENT_5:
                                me->SetEntry(NPC_HARRISON_JONES_1);
                                me->SetDisplayId(MODEL_HARRISON_JONES_1);
                                Talk(SAY_HARRISON_2);
                                _gongTimer = 12000;
                                _gongEvent = GONG_EVENT_6;
                                break;
                            case GONG_EVENT_6:
                                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_USE_STANDING);
                                Talk(SAY_HARRISON_3);
                                _gongTimer = 7000;
                                _gongEvent = GONG_EVENT_7;
                                break;
                            case GONG_EVENT_7:
                            {
                                if (GameObject* gate = me->GetMap()->GetGameObject(instance->GetData64(GO_MASSIVE_GATE)))
                                    gate->SetGoState(GO_STATE_ACTIVE);
                                _gongTimer = 2000;
                                _gongEvent = GONG_EVENT_8;
                                break;
                            }
                            case GONG_EVENT_8:
                            {
                                //                                DoCast(me, SPELL_STEALTH);
                                std::list<Creature*> targetList;
                                GetCreatureListWithEntryInGrid(targetList, me, NPC_AMANISHI_GUARDIAN, 26.0f);
                                if (!targetList.empty())
                                {
                                    for (std::list<Creature*>::const_iterator itr = targetList.begin(); itr != targetList.end(); ++itr)
                                    {
                                        if (Creature* ptarget = *itr)
                                        {
                                            ptarget->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 0, uint32(WEAPON_SPEAR));
                                            ptarget->CastWithDelay(2000, me, SPELL_COSMETIC_SPEAR_THROW, true);
                                        }
                                    }
                                }
                                me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 0, uint32(0));
                                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_ONESHOT_NONE);
                                me->GetMotionMaster()->MovePath(HARRISON_MOVE_3, false);
                                _gongTimer = 1000;
                                _gongEvent = 0;
                                break;
                            }
                            case GONG_EVENT_9:
                                me->GetMotionMaster()->MovePoint(0, 120.687f, 1674.0f, 42.0217f);
                                _gongTimer = 12000;
                                _gongEvent = GONG_EVENT_10;
                                break;
                            case GONG_EVENT_10:
                                me->SetFacingTo(1.59044f);
                                _gongEvent = 11;
                                _gongTimer = 6000;
                                break;
                            case GONG_EVENT_11:
                                instance->SetBossState(DATA_GONGEVENT, NOT_STARTED);
                                me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                                _gongEvent = 0;
                                _gongTimer = 1000;
                                break;
                        }
                    }
                    else
                    _gongTimer -= diff;
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_harrison_jonesAI(creature);
        }
};

class spell_banging_the_gong : public SpellScriptLoader
{
    public:
        spell_banging_the_gong() : SpellScriptLoader("spell_banging_the_gong") { }

        class spell_banging_the_gong_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_banging_the_gong_SpellScript);

            void Activate(SpellEffIndex index)
            {
                PreventHitDefaultEffect(index);
                GetHitGObj()->SendCustomAnim(0);
                //                if (Creature *c = GetHitGObj()->FindNearestCreature(NPC_HARRISON_JONES, 100))
                //    c->AI()->SetData(DATA_HIT_GONG, 1);
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_banging_the_gong_SpellScript::Activate, EFFECT_1, SPELL_EFFECT_ACTIVATE_OBJECT);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_banging_the_gong_SpellScript();
        }
};


void AddSC_zulaman()
{
    new npc_forest_frog();
    new npc_zulaman_hostage();
    new npc_harrison_jones();
    new spell_banging_the_gong();
}
