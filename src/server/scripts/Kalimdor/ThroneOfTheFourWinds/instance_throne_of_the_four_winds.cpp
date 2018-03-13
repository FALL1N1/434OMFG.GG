
#include "Player.h"
#include "InstanceScript.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "throne_of_the_four_winds.h"

#define EncounterCount 2

class instance_throne_of_the_four_winds : public InstanceMapScript
{
public:
    instance_throne_of_the_four_winds() : InstanceMapScript("instance_throne_of_the_four_winds", 754) { }

    struct instance_throne_of_the_four_winds_InstanceMapScript: public InstanceScript
    {
        instance_throne_of_the_four_winds_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
        {
            SetHeaders(DataHeader);
            uiAnshal = 0;
            uiNezir = 0;
            uiRohash = 0;
            uiAlakir = 0;
            killedBosses = 0;
            uiPlatform = 0;
            enrageTimer = 0;
            underMapTimer = 1000;
            canEnrage = false;
            deactivateSlipstream = false;
            SetBossNumber(EncounterCount);
        }

        void OnCreatureCreate(Creature* creature)
        {
            switch (creature->GetEntry())
            {
                case BOSS_ANSHAL:
                    uiAnshal = creature->GetGUID();
                    break;
                case BOSS_NEZIR:
                    uiNezir = creature->GetGUID();
                    break;
                case BOSS_ROHASH:
                    uiRohash = creature->GetGUID();
                    break;
                case BOSS_ALAKIR:
                    uiAlakir = creature->GetGUID();
                    break;
                case NPC_HURRICANE_TRIGGER:
                    creature->SetCanFly(true);
                    creature->SetDisableGravity(true);
                    creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
                    break;
                case NPC_SLIPSTREAM_ALAKIR:
                    if (GetBossState(DATA_CONCLAVE_OF_WIND_EVENT) == DONE || GetBossState(DATA_CONCLAVE_OF_WIND_EVENT) == DONE_HM)
                        creature->SetVisible(true);
                    else creature->SetVisible(false);
                    break;
                default:
                    break;
            }
        }

        void OnGameObjectCreate(GameObject* go)
        {
            switch (go->GetEntry())
            {
                case GOB_CENTER_PLATFORM:
                    uiPlatform = go->GetGUID();
                    break;
                default:
                    break;
            }
        }
        uint64 GetData64(uint32 identifier) const
        {
            switch (identifier)
            {
                case DATA_ANSHAL:
                    return uiAnshal;
                case DATA_NEZIR:
                    return uiNezir;
                case DATA_ROHASH:
                    return uiRohash;
                case DATA_ALAKIR:
                    return uiAlakir;
                case GOB_CENTER_PLATFORM:
                    return uiPlatform;
                default:
                    break;
            }
            return 0;
        }

        void SetData(uint32 type, uint32 data)
        {
            switch (type)
            {
                case DATA_KILLED_BOSSES:
                    killedBosses = data;
                    if (killedBosses == 3)
                    {
                        Map::PlayerList const& players = instance->GetPlayers();
                        for (Map::PlayerList::const_iterator i = players.begin(); i != players.end(); ++i)
                            if (Player* player = i->getSource())
                            {
                                if (Creature* anshal = instance->GetCreature(uiAnshal))
                                {
                                    anshal->LowerPlayerDamageReq(anshal->GetMaxHealth());
                                    player->Kill(anshal);
                                }
                                if (Creature* nezir = instance->GetCreature(uiNezir))
                                {
                                    nezir->LowerPlayerDamageReq(nezir->GetMaxHealth());
                                    player->Kill(nezir);
                                }
                                if (Creature* rohash = instance->GetCreature(uiRohash))
                                {
                                    rohash->LowerPlayerDamageReq(rohash->GetMaxHealth());
                                    player->Kill(rohash);
                                }
                                break;
                            }
                    }
                    break;
                case DATA_DEACTIVATE_SLIPSTREAM:
                    deactivateSlipstream = data;
                    break;
                default:
                    break;
            }
        }

        void OnPlayerEnter(Player *player)
        {
            if (instance->IsHeroic() && !IsDoneInHeroic(DATA_CONCLAVE_OF_WIND_EVENT))
                if (Creature* Alakir = instance->GetCreature(uiAlakir))
                    Alakir->SetVisible(false);

            player->ClearInCombat();
        }

        uint32 GetData(uint32 type) const
        {
            switch (type)
            {
                case DATA_KILLED_BOSSES:
                    return killedBosses;
                case DATA_DEACTIVATE_SLIPSTREAM:
                    return deactivateSlipstream;
                default:
                    break;
            }
            return 0;
        }

        bool SetBossState(uint32 data, EncounterState state)
        {
            if (!InstanceScript::SetBossState(data, state))
                return false;

            switch (data)
            {
                case DATA_CONCLAVE_OF_WIND_EVENT:
                    switch (state)
                    {
                        case DONE:
                        case DONE_HM:
                            DoUpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET, SPELL_WIND_BOSSES_ALL_DEAD);
                            DoCastSpellOnPlayers(SPELL_WIND_BOSSES_ALL_DEAD);
                            UpdateEncounterState(ENCOUNTER_CREDIT_CAST_SPELL, 88835, NULL);
                            if (Creature* anshal = instance->GetCreature(uiAnshal))
                                SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, anshal);
                            if (Creature* nezir = instance->GetCreature(uiNezir))
                                SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, nezir);
                            if (Creature* rohash = instance->GetCreature(uiRohash))
                                SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, rohash);
                            if (Creature* Alakir = instance->GetCreature(uiAlakir))
                                Alakir->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                            break;
                        case IN_PROGRESS:
                            canEnrage = true;
                            enrageTimer = 480000;
                            if (Creature* anshal = instance->GetCreature(uiAnshal))
                            {
                                SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, anshal);
                                anshal->SetInCombatWithZone();
                            }
                            if (Creature* nezir = instance->GetCreature(uiNezir))
                            {
                                SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, nezir);
                                nezir->SetInCombatWithZone();
                            }
                            if (Creature* rohash = instance->GetCreature(uiRohash))
                            {
                                SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, rohash);
                                rohash->SetInCombatWithZone();
                            }
                            break;
                        case NOT_STARTED:
                            canEnrage = false;
                            if (Creature* anshal = instance->GetCreature(uiAnshal))
                                SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, anshal);
                            if (Creature* nezir = instance->GetCreature(uiNezir))
                                SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, nezir);
                            if (Creature* rohash = instance->GetCreature(uiRohash))
                                SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, rohash);
                            break;
                        }
                        break;
                    case DATA_ALAKIR_EVENT:
                        break;
                    default:
                        break;
            }

            if (instance->IsHeroic() && !IsDoneInHeroic(DATA_CONCLAVE_OF_WIND_EVENT))
                if (Creature* Alakir = instance->GetCreature(uiAlakir))
                    Alakir->SetVisible(false);

            return true;
        }

        void Update(uint32 diff)
        {
            if (underMapTimer <= diff)
            {
                Map::PlayerList const& players = instance->GetPlayers();
                for (Map::PlayerList::const_iterator i = players.begin(); i != players.end(); ++i)
                    if (Player* player = i->getSource())
                    {
                        Position pos;
                        player->GetPosition(&pos);
                        if (player->GetPositionZ() <= 170.0f && !player->GetVehicle())
                        {
                            if (Creature *creature = instance->SummonCreature(45710, pos))
                            {
                                creature->AI()->SetGUID(player->GetGUID());
                                creature->GetMotionMaster()->MoveJump(player->GetPositionX(), player->GetPositionY(), 200, 10, 10, EVENT_JUMP);
                                player->AddAura(85269, player);
                                player->SetViewpoint(creature, true);
                            }
                        }
                    }
                underMapTimer = 500;
            }
            else
                underMapTimer -= diff;

            if (canEnrage)
            {
                if (enrageTimer <= diff)
                {
                    if (Creature* anshal = instance->GetCreature(uiAnshal))
                        anshal->CastSpell(anshal, SPELL_BERSERK, true);
                    if (Creature* nezir = instance->GetCreature(uiNezir))
                        nezir->CastSpell(nezir, SPELL_BERSERK, true);
                    if (Creature* rohash = instance->GetCreature(uiRohash))
                        rohash->CastSpell(rohash, SPELL_BERSERK, true);
                    canEnrage = false;
                }
                else enrageTimer -= diff;
            }
        }

    private:
        bool canEnrage;
        bool deactivateSlipstream;
        uint32 underMapTimer;
        uint32 enrageTimer;
        uint32 killedBosses;
        uint64 uiAnshal;
        uint64 uiNezir;
        uint64 uiRohash;
        uint64 uiAlakir;
        uint64 uiPlatform;
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_throne_of_the_four_winds_InstanceMapScript(map);
    }
};

void AddSC_instance_throne_of_the_four_winds()
{
    new instance_throne_of_the_four_winds();
}
