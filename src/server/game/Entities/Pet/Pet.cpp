/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#include "Common.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "WorldPacket.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "Pet.h"
#include "Formulas.h"
#include "SpellAuras.h"
#include "SpellAuraEffects.h"
#include "CreatureAI.h"
#include "Unit.h"
#include "Util.h"
#include "Group.h"
#include "Opcodes.h"
#include "WorldSession.h"
#include "Transport.h"

#define PET_XP_FACTOR 0.05f

Pet::Pet(Player* owner, PetType type) : Guardian(NULL, owner, true),
    m_usedTalentCount(0), m_removed(false), m_owner(owner),
    m_petType(type), m_duration(0),
    m_auraRaidUpdateMask(0), m_loading(false), m_declinedname(NULL), _isAurasLoading(false)
{
    m_unitTypeMask |= UNIT_MASK_PET;
    if (type == HUNTER_PET)
        m_unitTypeMask |= UNIT_MASK_HUNTER_PET;

    if (!(m_unitTypeMask & UNIT_MASK_CONTROLABLE_GUARDIAN))
    {
        m_unitTypeMask |= UNIT_MASK_CONTROLABLE_GUARDIAN;
        InitCharmInfo();
    }

    m_name = "Pet";
    m_regenTimer = PET_FOCUS_REGEN_INTERVAL;
}

Pet::~Pet()
{
    delete m_declinedname;
}

void Pet::AddToWorld()
{
    ///- Register the pet for guid lookup
    if (!IsInWorld())
    {
        ///- Register the pet for guid lookup
        sObjectAccessor->AddObject(this);
        Unit::AddToWorld();
        AIM_Initialize();
    }

    // Prevent stuck pets when zoning. Pets default to "follow" when added to world
    // so we'll reset flags and let the AI handle things
    if (GetCharmInfo() && GetCharmInfo()->HasCommandState(COMMAND_FOLLOW))
    {
        GetCharmInfo()->SetIsCommandAttack(false);
        GetCharmInfo()->SetIsCommandFollow(false);
        GetCharmInfo()->SetIsAtStay(false);
        GetCharmInfo()->SetIsFollowing(false);
        GetCharmInfo()->SetIsReturning(false);
    }
}

void Pet::RemoveFromWorld()
{
    ///- Remove the pet from the accessor
    if (IsInWorld())
    {
        ///- Don't call the function for Creature, normal mobs + totems go in a different storage
        Unit::RemoveFromWorld();
        sObjectAccessor->RemoveObject(this);
    }
}


bool Pet::LoadPet(Player* owner, PetData* t_pet, bool current)
{
    m_loading = true;

    if (!t_pet || t_pet->_state == DATA_REMOVED)
    {
        m_loading = false;
        return false;
    }

    if (!t_pet->entry)
        return false;

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(t_pet->summon_spell_id);

    bool is_temporary_summoned = spellInfo && spellInfo->GetDuration() > 0;

    if (current && t_pet->pet_type == HUNTER_PET)
    {
        CreatureTemplate const* creatureInfo = sObjectMgr->GetCreatureTemplate(t_pet->entry);
        if (!creatureInfo || !creatureInfo->isTameable(owner->CanTameExoticPets()))
            return false;
    }

    if (current && owner->IsPetNeedBeTemporaryUnsummoned())
    {
        owner->SetTemporaryUnsummonedPetNumber(t_pet->id);
        return false;
    }

    Map* map = owner->GetMap();
    uint32 guid = sObjectMgr->GenerateLowGuid(HIGHGUID_PET);
    if (!Create(guid, map, owner->GetPhaseMask(), t_pet->entry, t_pet->id))
        return false;

    setPetType(t_pet->pet_type);
    setFaction(owner->getFaction());
    if (t_pet->pet_type != HUNTER_PET)
        SetUInt32Value(UNIT_CREATED_BY_SPELL, t_pet->summon_spell_id);
    else
        SetUInt32Value(UNIT_CREATED_BY_SPELL, 0);

    CreatureTemplate const* cinfo = GetCreatureTemplate();
    if (cinfo->type == CREATURE_TYPE_CRITTER)
    {

        float px, py, pz;
        float po = owner->GetOrientation();
        owner->GetClosePoint(px, py, pz, GetObjectSize(), PET_FOLLOW_DIST, GetFollowAngle());
        Relocate(px, py, pz, po);

        if (!IsPositionValid())
        {
            TC_LOG_ERROR("entities.pet", "Pet (guidlow %d, entry %d) not loaded. Suggested coordinates isn't valid (X: %f Y: %f)",
                GetGUIDLow(), GetEntry(), GetPositionX(), GetPositionY());
            return false;
        }

        Transport *transport = owner->GetTransport();
        if (transport)
        {
            transport->CalculatePassengerOffset(px, py, pz, po);
            SetTransport(transport);
            m_movementInfo.t_guid = transport->GetGUID();
            m_movementInfo.t_pos.Relocate(px, py, pz, po);
            transport->AddPassenger(this);
        }

        map->AddToMap(this->ToCreature());
        return true;
    }

    m_charmInfo->SetPetNumber(t_pet->id, IsPermanentPetFor(owner));

    SetDisplayId(t_pet->modelid);
    SetNativeDisplayId(t_pet->modelid);
    uint32 petlevel = t_pet->level;
    SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
    SetName(t_pet->name);

    switch (getPetType())
    {
        case SUMMON_PET:
            petlevel = owner->getLevel();
            if (!IsPetGhoul())
                SetUInt32Value(UNIT_FIELD_BYTES_0, 0x800); // class = mage
            SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
                                                            // this enables popup window (pet dismiss, cancel)
            break;
        case HUNTER_PET:
            SetUInt32Value(UNIT_FIELD_BYTES_0, 0x02020100); // class = warrior, gender = none, power = focus
            SetSheath(SHEATH_STATE_MELEE);
            SetByteFlag(UNIT_FIELD_BYTES_2, 2, t_pet->renamed ? UNIT_CAN_BE_ABANDONED : UNIT_CAN_BE_RENAMED | UNIT_CAN_BE_ABANDONED);

            SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
                                                            // this enables popup window (pet abandon, cancel)
            setPowerType(POWER_FOCUS);
            break;
        default:
            if (!IsPetGhoul())
                TC_LOG_ERROR("entities.pet", "Pet have incorrect type (%u) for pet loading.", getPetType());
            break;
    }

    SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, uint32(time(NULL))); // cast can't be helped here
    SetCreatorGUID(owner->GetGUID());

    InitStatsForLevel(petlevel);
    SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, t_pet->exp);

    SynchronizeLevelWithOwner();

    // Set pet's position after setting level, its size depends on it
    float px, py, pz;
    owner->GetClosePoint(px, py, pz, GetObjectSize(), PET_FOLLOW_DIST, GetFollowAngle());
    Relocate(px, py, pz, owner->GetOrientation());
    if (!IsPositionValid())
    {
        TC_LOG_ERROR("entities.pet", "Pet (guidlow %d, entry %d) not loaded. Suggested coordinates isn't valid (X: %f Y: %f)",
            GetGUIDLow(), GetEntry(), GetPositionX(), GetPositionY());
        return false;
    }

    SetReactState(t_pet->reactstate);
    SetCanModifyStats(true);

    // Send fake summon spell cast - this is needed for correct cooldown application for spells
    // Example: 46584 - without this cooldown (which should be set always when pet is loaded) isn't set clientside
    // TODO: pets should be summoned from real cast instead of just faking it?
    if (t_pet->summon_spell_id)
    {
        WorldPacket data(SMSG_SPELL_GO, (8+8+4+4+2));
        data.append(owner->GetPackGUID());
        data.append(owner->GetPackGUID());
        data << uint8(0);
        data << uint32(t_pet->summon_spell_id);
        data << uint32(256); // CAST_FLAG_UNKNOWN3
        data << uint32(0);
        data << uint32(getMSTime());
        owner->SendMessageToSet(&data, true);
    }

    if (t_pet->slot < 0 || t_pet->slot > PET_SLOT_HUNTER_LAST)
        t_pet->slot = 0;

    owner->SetMinion(this, true, PetSlot(t_pet->slot));
    map->AddToMap(this->ToCreature());

    InitTalentForLevel();                                   // set original talents points before spell loading

    uint32 timediff = uint32(time(NULL) - t_pet->savetime);
    _LoadAuras(t_pet,timediff);

    // load action bar, if data broken will fill later by default spells.
    if (!is_temporary_summoned)
    {
        m_charmInfo->LoadPetActionBar(t_pet->abdata);

        _LoadSpells(t_pet);
        InitTalentForLevel();                               // re-init to check talent count
        _LoadSpellCooldowns(t_pet);
        LearnPetPassives();
        InitLevelupSpellsForLevel();
        if (map->IsBattleArena())
            RemoveArenaAuras();

        CastPetAuras(current);
    }

    if (getPetType() == SUMMON_PET && !current)              //all (?) summon pets come with full health when called, but not when they are current
    {
        SetPower(POWER_MANA, GetMaxPower(POWER_MANA));
        SetFullHealth();
    }
    else
    {
        uint32 savedhealth = t_pet->curhealth;
        uint32 savedmana = t_pet->curmana;
        if (!savedhealth && getPetType() == HUNTER_PET)
        {
            setDeathState(JUST_DIED);
            t_pet->state = PET_STATE_DEAD;
        }
        else
        {
            SetHealth(savedhealth > GetMaxHealth() ? GetMaxHealth() : savedhealth);
            SetPower(POWER_MANA, savedmana > uint32(GetMaxPower(POWER_MANA)) ? GetMaxPower(POWER_MANA) : savedmana);
        }
    }

    CleanupActionBar();                                     // remove unknown spells from action bar after load

    TC_LOG_DEBUG("entities.pet", "New Pet has guid %u", GetGUIDLow());

    owner->PetSpellInitialize();

    if (owner->GetGroup())
        owner->SetGroupUpdateFlag(GROUP_UPDATE_PET);

    owner->SendTalentsInfoData(true);

    if (getPetType() == HUNTER_PET)
    {
        PreparedStatement* stmt;
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PET_DECLINED_NAME);
        stmt->setUInt32(0, owner->GetGUIDLow());
        stmt->setUInt32(1, GetCharmInfo()->GetPetNumber());
        PreparedQueryResult result = CharacterDatabase.Query(stmt);

        if (result)
        {
            delete m_declinedname;
            m_declinedname = new DeclinedName;
            Field* fields2 = result->Fetch();
            for (uint8 i = 0; i < MAX_DECLINED_NAME_CASES; ++i)
            {
                m_declinedname->name[i] = fields2[i].GetString();
            }
        }
    }

    // set last used pet number (for use in BG's)
    if (owner->GetTypeId() == TYPEID_PLAYER && isControlled() && !isTemporarySummoned() && (getPetType() == SUMMON_PET || getPetType() == HUNTER_PET))
    {
        owner->ToPlayer()->SetLastPetId(t_pet->id);
        owner->ToPlayer()->SetLastPetSlot(t_pet->slot);
        owner->ToPlayer()->SetLastPetNumber(t_pet->summon_spell_id);
        owner->ToPlayer()->m_arenaSpectatorFlags |= ASPEC_FLAG_PET;
    }
    m_loading = false;

    return true;
}

void Pet::SavePet(PetSlot mode, bool logout)
{
    if (!GetEntry())
        return;

    // save only fully controlled creature
    if (!isControlled())
        return;

    // not save not player pets
    if (!IS_PLAYER_GUID(GetOwnerGUID()))
        return;

    Player* owner = GetOwner()->ToPlayer();
    if (!owner)
        return;

    PetData* t_pet = owner->GetPetDatabyGuid(m_charmInfo->GetPetNumber());
    if (t_pet && mode == PET_SLOT_DELETED)
    {
        t_pet->_state = DATA_REMOVED;
        return;
    }
    else if (t_pet)
        t_pet->_state = DATA_CHANGED;
    else
    {
        // Newly created pet (tamed)
        t_pet = new PetData();
        t_pet->_state = DATA_NEW;
        owner->m_Stables[GetGUIDLow()] = t_pet;
    }

    uint32 curhealth = GetHealth();
    uint32 curmana = GetPower(POWER_MANA);

    // stable and not in slot saves
    if ((mode > PET_SLOT_HUNTER_LAST && getPetType() == HUNTER_PET) || mode == PET_SLOT_NOT_IN_SLOT)
        RemoveAllAuras();

    // save auras before possibly removing them
    _SaveAuras(t_pet);
    _SaveSpells(t_pet);
    _SaveSpellCooldowns(t_pet);

    if (getPetType() == SUMMON_PET)
        mode = logout ? PET_SLOT_DEFAULT : PET_SLOT_OTHER_PET;
    else if (getPetType() == HUNTER_PET && mode == PET_SLOT_ACTUAL_PET_SLOT)
    {
        mode = t_pet->_state == DATA_NEW ? PetSlot(owner->_currentPetSlot) : PetSlot(t_pet->slot);
        if (mode < 0 || mode  > PET_SLOT_HUNTER_LAST)
            TC_LOG_ERROR("misc", "Pet is being saved to an invalid slot %i", owner->_currentPetSlot);
    }

    // current/stable/not_in_slot

    std::ostringstream ss_abdata;
    std::string name = m_name;

    for (uint32 i = ACTION_BAR_INDEX_START; i < ACTION_BAR_INDEX_END; ++i)
    {
        ss_abdata << uint32(m_charmInfo->GetActionBarEntry(i)->GetType()) << ' '
                    << uint32(m_charmInfo->GetActionBarEntry(i)->GetAction()) << ' ';
    };

    t_pet->id                  = m_charmInfo->GetPetNumber();
    t_pet->entry               = GetEntry();
    t_pet->owner               = GetGUIDLow();
    t_pet->modelid             = GetNativeDisplayId();
    t_pet->level               = uint32(getLevel());
    t_pet->exp                 = GetUInt32Value(UNIT_FIELD_PETEXPERIENCE);
    t_pet->reactstate          = GetReactState();
    t_pet->slot                = uint32(mode);
    t_pet->name                = name.c_str();
    t_pet->renamed             = uint32(HasByteFlag(UNIT_FIELD_BYTES_2, 2, UNIT_CAN_BE_RENAMED) ? 0 : 1);
    t_pet->curhealth           = curhealth;
    t_pet->curmana             = curmana;
    t_pet->abdata              = ss_abdata.str();
    t_pet->summon_spell_id     = GetUInt32Value(UNIT_CREATED_BY_SPELL);
    t_pet->savetime            = time(NULL);
    t_pet->pet_type            = getPetType();

    if (curhealth <= 0)
        t_pet->state = PET_STATE_DEAD;
    else
        t_pet->state = PET_STATE_ALIVE;
}

void Pet::DeleteFromDB(uint32 guidlow)
{
    SQLTransaction trans = CharacterDatabase.BeginTransaction();

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_PET_BY_ID);
    stmt->setUInt32(0, guidlow);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_PET_DECLINEDNAME);
    stmt->setUInt32(0, guidlow);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PET_AURAS);
    stmt->setUInt32(0, guidlow);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PET_SPELLS);
    stmt->setUInt32(0, guidlow);
    trans->Append(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PET_SPELL_COOLDOWNS);
    stmt->setUInt32(0, guidlow);
    trans->Append(stmt);

    CharacterDatabase.CommitTransaction(trans);
}

void Pet::setDeathState(DeathState s)                       // overwrite virtual Creature::setDeathState and Unit::setDeathState
{
    Creature::setDeathState(s);

    if (s == JUST_DIED)
    {
        // Demonic Rebirth
        if (getPetType() == SUMMON_PET && GetOwner() && GetOwner()->getClass() == CLASS_WARLOCK)
        {
            if (!GetOwner()->HasAura(89140)) // Demonic Rebirth Marker
            {
                if (AuraEffect* aurEff = GetOwner()->GetDummyAuraEffect(SPELLFAMILY_WARLOCK, 1981, EFFECT_0)) // Demonic Rebirth
                {
                    GetOwner()->CastCustomSpell(88448, SPELLVALUE_BASE_POINT0, -(aurEff->GetAmount()), GetOwner(), true, NULL, aurEff);
                    GetOwner()->CastSpell(GetOwner(), 89140, true);
                }
            }
        }

        if (isControlled())
            if (GetOwner())
                GetOwner()->m_arenaSpectatorFlags |= ASPEC_FLAG_PHP;
    }

    if (getDeathState() == CORPSE)
    {
        if (getPetType() == HUNTER_PET)
        {
            // pet corpse non lootable and non skinnable
            SetUInt32Value(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_NONE);
            RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
            //SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
        }
    }
    else if (getDeathState() == ALIVE)
    {
        //RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
        CastPetAuras(true);
    }
}

void Pet::Update(uint32 diff)
{
    if (m_removed)                                           // pet already removed, just wait in remove queue, no updates
        return;

    if (m_loading)
        return;

    if (getPetType() == SUMMON_PET && GetOwner() && GetOwner()->getClass() == CLASS_WARLOCK)
    {
        if (AuraEffect* aurEff = GetOwner()->GetDummyAuraEffect(SPELLFAMILY_WARLOCK, 3220, EFFECT_0)) // Demonic Pact
        {
            if (!HasAura(53646)) // Demonic Pact raid buff
                CastSpell(this, 53646, true);
        }
        else if (HasAura(53646))
            RemoveAurasDueToSpell(53646);
    }

    switch (m_deathState)
    {
        case CORPSE:
        {
            if (getPetType() != HUNTER_PET || m_corpseRemoveTime <= time(NULL))
            {
                Remove(PET_SLOT_ACTUAL_PET_SLOT);               //hunters' pets never get removed because of death, NEVER!
                return;
            }
            break;
        }
        case ALIVE:
        {
            // unsummon pet that lost owner
            Player* owner = GetOwner();
            if (!owner || (!IsWithinDistInMap(owner, GetMap()->GetVisibilityRange()) && !isPossessed()) || (isControlled() && !owner->GetPetGUID()))
            //if (!owner || (!IsWithinDistInMap(owner, GetMap()->GetVisibilityDistance()) && (owner->GetCharmGUID() && (owner->GetCharmGUID() != GetGUID()))) || (isControlled() && !owner->GetPetGUID()))
            {
                Remove(PET_SLOT_ACTUAL_PET_SLOT, true);
                return;
            }

            if (isControlled())
            {
                if (owner->GetPetGUID() != GetGUID())
                {
                    TC_LOG_ERROR("entities.pet", "Pet %u is not pet of owner %s, removed", GetEntry(), m_owner->GetName().c_str());
                    Remove(PET_SLOT_ACTUAL_PET_SLOT);
                    return;
                }
            }

            if (m_duration > 0)
            {
                if (uint32(m_duration) > diff)
                    m_duration -= diff;
                else
                {
                    Remove(PET_SLOT_ACTUAL_PET_SLOT);
                    return;
                }
            }

            //regenerate focus for hunter pets or energy for deathknight's ghoul
            if (m_regenTimer)
            {
                if (m_regenTimer > diff)
                    m_regenTimer -= diff;
                else
                {
                    switch (getPowerType())
                    {
                        case POWER_FOCUS:
                            Regenerate(POWER_FOCUS);
                            m_regenTimer += PET_FOCUS_REGEN_INTERVAL - diff;
                            if (!m_regenTimer) ++m_regenTimer;

                            // Reset if large diff (lag) causes focus to get 'stuck'
                            if (m_regenTimer > PET_FOCUS_REGEN_INTERVAL)
                                m_regenTimer = PET_FOCUS_REGEN_INTERVAL;

                            break;

                        // in creature::update
                        //case POWER_ENERGY:
                        //    Regenerate(POWER_ENERGY);
                        //    m_regenTimer += CREATURE_REGEN_INTERVAL - diff;
                        //    if (!m_regenTimer) ++m_regenTimer;
                        //    break;
                        default:
                            m_regenTimer = 0;
                            break;
                    }
                }
            }
            break;
        }
        default:
            break;
    }
    Creature::Update(diff);
}

void Pet::Remove(PetSlot mode, bool returnreagent)
{
    m_owner->RemovePet(this, mode, returnreagent);
}

void Pet::GivePetLevel(uint8 level)
{
    if (!level || level == getLevel())
        return;

    if (getPetType() == HUNTER_PET)
    {
        SetUInt32Value(UNIT_FIELD_PETEXPERIENCE, 0);
        SetUInt32Value(UNIT_FIELD_PETNEXTLEVELEXP, 0);
    }

    InitStatsForLevel(level);
    InitLevelupSpellsForLevel();
    InitTalentForLevel();
}

bool Pet::CreateBaseAtCreature(Creature* creature)
{
    ASSERT(creature);

    if (!CreateBaseAtTamed(creature->GetCreatureTemplate(), creature->GetMap(), creature->GetPhaseMask()))
        return false;

    Relocate(creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ(), creature->GetOrientation());

    if (!IsPositionValid())
    {
        TC_LOG_ERROR("entities.pet", "Pet (guidlow %d, entry %d) not created base at creature. Suggested coordinates isn't valid (X: %f Y: %f)",
            GetGUIDLow(), GetEntry(), GetPositionX(), GetPositionY());
        return false;
    }

    CreatureTemplate const* cinfo = GetCreatureTemplate();
    if (!cinfo)
    {
        TC_LOG_ERROR("entities.pet", "CreateBaseAtCreature() failed, creatureInfo is missing!");
        return false;
    }

    SetDisplayId(creature->GetDisplayId());

    if (CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(cinfo->family))
        SetName(cFamily->Name);
    else
        SetName(creature->GetNameForLocaleIdx(sObjectMgr->GetDBCLocaleIndex()));

    return true;
}

bool Pet::CreateBaseAtCreatureInfo(CreatureTemplate const* cinfo, Unit* owner)
{
    if (!CreateBaseAtTamed(cinfo, owner->GetMap(), owner->GetPhaseMask()))
        return false;

    if (CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(cinfo->family))
        SetName(cFamily->Name);

    Relocate(owner->GetPositionX(), owner->GetPositionY(), owner->GetPositionZ(), owner->GetOrientation());

    return true;
}

bool Pet::CreateBaseAtTamed(CreatureTemplate const* cinfo, Map* map, uint32 phaseMask)
{
    TC_LOG_DEBUG("entities.pet", "Pet::CreateBaseForTamed");
    uint32 guid=sObjectMgr->GenerateLowGuid(HIGHGUID_PET);
    uint32 pet_number = sObjectMgr->GeneratePetNumber();
    if (!Create(guid, map, phaseMask, cinfo->Entry, pet_number))
        return false;

    setPowerType(POWER_FOCUS);
    SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, 0);
    SetUInt32Value(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_NONE);

    if (cinfo->type == CREATURE_TYPE_BEAST)
    {
        SetUInt32Value(UNIT_FIELD_BYTES_0, 0x02020100);
        SetSheath(SHEATH_STATE_MELEE);
        SetByteFlag(UNIT_FIELD_BYTES_2, 2, UNIT_CAN_BE_RENAMED | UNIT_CAN_BE_ABANDONED);
    }

    return true;
}

// TODO: Move stat mods code to pet passive auras
bool Guardian::InitStatsForLevel(uint8 petlevel)
{
    CreatureTemplate const* cinfo = GetCreatureTemplate();
    ASSERT(cinfo);

    SetLevel(petlevel);

    //Determine pet type
    PetType petType = MAX_PET_TYPE;
    if (isPet() && m_owner->GetTypeId() == TYPEID_PLAYER)
    {
        if ((m_owner->getClass() == CLASS_WARLOCK)
            || (m_owner->getClass() == CLASS_SHAMAN)        // Fire Elemental
            || (m_owner->getClass() == CLASS_DEATH_KNIGHT)  // Risen Ghoul
            || (m_owner->getClass() == CLASS_MAGE)) // Water elemental
            petType = SUMMON_PET;
        else if (m_owner->getClass() == CLASS_HUNTER)
        {
            petType = HUNTER_PET;
            m_unitTypeMask |= UNIT_MASK_HUNTER_PET;
        }
        else
            TC_LOG_ERROR("entities.pet", "Unknown type pet %u is summoned by player class %u", GetEntry(), m_owner->getClass());
    }

    uint32 creature_ID = (petType == HUNTER_PET) ? 1 : cinfo->Entry;

    SetMeleeDamageSchool(SpellSchools(cinfo->dmgschool));

    SetModifierValue(UNIT_MOD_ARMOR, BASE_VALUE, float(petlevel*50));

    // @TODO: sniff guardian attack time im nearly sure that not all guardians have 2sec attack time (like shadowfiend)
    if (m_owner->getClass() == CLASS_PRIEST)
    {
        SetAttackTime(BASE_ATTACK, 1500);
        SetAttackTime(OFF_ATTACK, 1500);
        SetAttackTime(RANGED_ATTACK, 1500);
    }
    else
    {
        SetAttackTime(BASE_ATTACK, BASE_ATTACK_TIME);
        SetAttackTime(OFF_ATTACK, BASE_ATTACK_TIME);
        SetAttackTime(RANGED_ATTACK, BASE_ATTACK_TIME);
    }

    SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);
    SetFloatValue(UNIT_MOD_CAST_HASTE, 1.0f);

	// Pet scale
    CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(cinfo->family);
	if (cFamily && petType == HUNTER_PET)
    {
        float scale, minscale, maxscale, maxlevel;
         minscale = 0.6f;
         maxscale = 0.8f; // Additional 0.4f total scale for exotic pets
 
         uint32 levelCap = (cinfo->type_flags & CREATURE_TYPEFLAGS_EXOTIC) ? 85 : 70;
         maxlevel = getLevel() > levelCap ? levelCap : getLevel();
 
         if (cinfo->type_flags & CREATURE_TYPEFLAGS_EXOTIC)
         {
             scale = minscale + 0.4f + (maxlevel * ((maxscale - minscale) / maxlevel));
 
             if (scale < 0.6f)
                 scale = 0.6f;
         }
        else
		{
			scale = minscale + (maxlevel * ((maxscale - minscale) / maxlevel));

			if (scale < 0.6f)
				scale = 0.6f;
		}

		 maxlevel = getLevel();

        if (m_owner->HasAura(57870)) // Glyph of Lesser Proportion
            scale *= 0.85f;

        SetObjectScale(scale);
    }

    // Resistance
    for (uint8 i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
        SetModifierValue(UnitMods(UNIT_MOD_RESISTANCE_START + i), BASE_VALUE, float(cinfo->resistance[i]));

    //health, mana, armor and resistance
    PetLevelInfo const* pInfo = sObjectMgr->GetPetLevelInfo(creature_ID, petlevel);
    if (pInfo)                                      // exist in DB
    {
        SetCreateHealth(pInfo->health);
        if (petType != HUNTER_PET) //hunter pet use focus
            SetCreateMana(pInfo->mana);

        if (pInfo->armor > 0)
            SetModifierValue(UNIT_MOD_ARMOR, BASE_VALUE, float(pInfo->armor));

        for (uint8 stat = 0; stat < MAX_STATS; ++stat)
            SetCreateStat(Stats(stat), float(pInfo->stats[stat]));

        SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, pInfo->mindmg);
        SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, pInfo->maxdmg);
    }
    else                                            // not exist in DB, use some default fake data
    {
        // remove elite bonuses included in DB values
        CreatureBaseStats const* stats = sObjectMgr->GetCreatureBaseStats(petlevel, cinfo->unit_class);
        SetCreateHealth(stats->BaseHealth[cinfo->expansion]);
        SetCreateMana(stats->BaseMana);

        SetCreateStat(STAT_STRENGTH, 22);
        SetCreateStat(STAT_AGILITY, 22);
        SetCreateStat(STAT_STAMINA, 25);
        SetCreateStat(STAT_INTELLECT, 28);
        SetCreateStat(STAT_SPIRIT, 27);

        SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petlevel - (petlevel / 4)));
        SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petlevel + (petlevel / 4)));
    }
    if (GetEntry() != 45322 && GetEntry() != 44214)
        SetSpellBonusDamage(0);

    switch (petType)
    {
        case SUMMON_PET:
        {
            switch (GetEntry())
            {
                case 510: // mage Water Elemental
                {
                    if (Player* owner = m_owner->ToPlayer())
                    {
                        m_modSpellHitChance += owner->GetRatingBonusValue(CR_HIT_SPELL);
                        m_modSpellHitChance += owner->GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_HIT_CHANCE);
                        owner->SetUInt32Value(PLAYER_PET_SPELL_POWER, int32(owner->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_FROST) * 0.4f));
                    }
                    SetCreateHealth(m_owner->GetMaxHealth() / 2);
                    SetCreateMana(m_owner->GetMaxPower(POWER_MANA)/ 2);
                    break;
                }
            }
            break;
        }
        default:
        {
            switch (GetEntry())
            {
                case 46506: // Melee Guardian
                {
                    float mindamage, maxdamage;
                    m_owner->ToPlayer()->CalculateMinMaxDamage(BASE_ATTACK, false, false, mindamage, maxdamage);
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, mindamage);
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, maxdamage);
                    break;
                }
                case 1964: //force of nature
                {
                    if (Player* owner = m_owner->ToPlayer())
                    {
                        int32 bonusDmg = int32(owner->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_NATURE) * 0.15f);
                        SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, GetWeaponDamageRange(BASE_ATTACK, MINDAMAGE) + bonusDmg);
                        SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, GetWeaponDamageRange(BASE_ATTACK, MAXDAMAGE) + bonusDmg);
                        m_modMeleeHitChance += owner->GetRatingBonusValue(CR_HIT_SPELL);
                        m_modMeleeHitChance += owner->GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_HIT_CHANCE);
                        SetCreateHealth(m_owner->CountPctFromMaxHealth(10));
                    }
                    break;
                }
                case 53432: // burning treant
                {
                    if (Player* owner = m_owner->ToPlayer())
                    {
                        m_modSpellHitChance += owner->GetRatingBonusValue(CR_HIT_SPELL);
                        m_modSpellHitChance += owner->GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_HIT_CHANCE);
                    }
                    break;
                }
                case 15352: //earth elemental 36213
                {
                    SetMaxHealth(m_owner->GetMaxHealth());
                    break;
                }
                case 15438: //fire elemental
                {
                    SetCreateMana(pInfo ? pInfo->mana : 28 + 10 * petlevel);
                    SetMaxHealth(m_owner->CountPctFromMaxHealth(75));
                    if (Player* owner = m_owner->ToPlayer())
                    {
                        SetSpellBonusDamage(owner->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_MAGIC));
                        m_modSpellHitChance += owner->GetRatingBonusValue(CR_HIT_SPELL);
                        m_modSpellHitChance += owner->GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_HIT_CHANCE);
                    }
                    break;
                }
                case 19668: // Shadowfiend
                {
                    if (!pInfo)
                    {
                        SetCreateMana(28 + 10*petlevel);
                        SetCreateHealth(28 + 30*petlevel);
                    }
                    int32 bonusDmg = (int32(m_owner->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_SHADOW)* 0.375f));
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, GetWeaponDamageRange(BASE_ATTACK, MINDAMAGE) + bonusDmg);
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, GetWeaponDamageRange(BASE_ATTACK, MAXDAMAGE) + bonusDmg);
                    break;
                }
                case 19833: //Snake Trap - Venomous Snake
                {
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float((petlevel / 2) - 25));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float((petlevel / 2) - 18));
                    break;
                }
                case 19921: //Snake Trap - Viper
                {
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petlevel / 2 - 10));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petlevel / 2));
                    break;
                }
                case 29264: // Feral Spirit
                {
                    if (!pInfo)
                        SetCreateHealth(30*petlevel);

                    float dmg_multiplier = 0.50f;
                    if (m_owner->GetAuraEffect(63271, 0)) // Glyph of Feral Spirit
                        dmg_multiplier = 0.80f;

                    SetSpellBonusDamage(int32(m_owner->GetTotalAttackPowerValue(BASE_ATTACK) * dmg_multiplier));

                    // 14AP == 1dps, wolf's strike speed == 1.5s so dmg = AP / 14 * 1.5
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, m_owner->GetTotalAttackPowerValue(BASE_ATTACK) * dmg_multiplier / 14);
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, m_owner->GetTotalAttackPowerValue(BASE_ATTACK) * dmg_multiplier / 14);

                    SetModifierValue(UNIT_MOD_ARMOR, BASE_VALUE, float(m_owner->GetArmor()) * 0.40f);  //  Bonus Armor (35% of player armor)
                    SetModifierValue(UNIT_MOD_STAT_STAMINA, BASE_VALUE, float(m_owner->GetStat(STAT_STAMINA)) * 0.40f);  //  Bonus Stamina (40% of player stamina)
                    if (!HasAura(58877))//prevent apply twice for the 2 wolves
                        AddAura(58877, this);//Spirit Hunt, passive, Spirit Wolves' attacks heal them and their master for 150% of damage done.
                    if (!HasAura(61783))        // prevent apply twice for the 2 wolves
                        AddAura(61783, this);   // Feral Pet Scaling
                    break;
                }
                case 31216: // Mirror Image
                {
                    if (!pInfo)
                    {
                        SetCreateMana(28 + 30 * petlevel);
                        SetCreateHealth(28 + 10 * petlevel);
                    }
                    if (Player* owner = m_owner->ToPlayer())
                    {
                        m_modSpellHitChance += owner->GetRatingBonusValue(CR_HIT_SPELL);
                        m_modSpellHitChance += owner->GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_HIT_CHANCE);
                        SetSpellBonusDamage(int32(owner->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_FROST) * 0.33f));
                        SetDisplayId(owner->GetDisplayId());
                    }
                    break;
                }
                case 53438: // t12 mirror image should only scale with hit rating
                {
                    if (Player* owner = m_owner->ToPlayer())
                    {
                        m_modSpellHitChance += owner->GetRatingBonusValue(CR_HIT_SPELL);
                        m_modSpellHitChance += owner->GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_HIT_CHANCE);
                    }
                    break;
                }
                case 27829: // Ebon Gargoyle
                {
                    if (!pInfo)
                        SetCreateHealth(m_owner->CountPctFromMaxHealth(70));

                    if(Player* owner = m_owner->ToPlayer())
                    {
                        float bonus = owner->GetRatingBonusValue(CR_HASTE_MELEE);
                        bonus += owner->GetTotalAuraModifier(SPELL_AURA_MOD_MELEE_HASTE_3) +
                        owner->GetTotalAuraModifier(SPELL_AURA_MOD_MELEE_RANGED_HASTE);
                        ApplyCastTimePercentMod(bonus, true);
                    }
                    SetSpellBonusDamage(int32(GetOwner()->GetTotalAttackPowerValue(BASE_ATTACK)));
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(petlevel - (petlevel / 4)));
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(petlevel + (petlevel / 4)));
                    break;
                }
                case 28017: // Bloodworms
                {
                    SetCreateHealth(m_owner->CountPctFromMaxHealth(18));
                    SetSpellBonusDamage(int32(m_owner->GetTotalAttackPowerValue(BASE_ATTACK) * 0.006f));
                    float minDamage = m_owner->GetTotalAttackPowerValue(BASE_ATTACK) * 0.05f;
                    float maxDamage = m_owner->GetTotalAttackPowerValue(BASE_ATTACK) * 0.05f;
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, minDamage);
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, maxDamage);
                    RemoveAurasDueToSpell(50453);
                    CastSpell(this, 50453, true);
                    break;
                }
                case 27893:             // Dancing Rune Weapon
                {
                    SetRedirectThreat(m_owner->GetGUID(), 100);
                    RemoveAurasDueToSpell(51906);
                    CastSpell(this, 51906, true);
                    break;
                }
                case 89:                // Infernal
                {
                    if (m_owner->GetTypeId() == TYPEID_PLAYER) // infernal get 100% of owners spell, Immolation has his own coef.
                        SetSpellBonusDamage(int32(m_owner->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_SPELL)));

                    float mod = 1;
                    if (petlevel < 60)
                        mod = 5 + (petlevel - 50) / 4;
                    if (petlevel < 70)
                        mod = 10 + (petlevel - 70) / 4;
                    else if (petlevel <= 80)
                        mod = 15 + (petlevel - 80) / 2;
                    else
                        mod = 20 + (petlevel - 85) / 2;
                    if (mod < 0)
                        mod = 0;
                    float minDamage = (petlevel - (petlevel / 4)) * mod;
                    float maxDamage = (petlevel + (petlevel / 4)) * mod;
                    float attackPower = ((minDamage + maxDamage) /4) * 7;
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, minDamage);
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, maxDamage);
                    SetModifierValue(UNIT_MOD_ATTACK_POWER, BASE_VALUE, attackPower);
                    SetCreateHealth(m_owner->CountPctFromMaxHealth(40));
                    break;
                }
                case 11859:                 // Doomguard
                {
                    if (!pInfo)                                      // not exist in DB
                    {
                        // remove elite bonuses included in DB values
                        CreatureTemplate const *cinfo = GetCreatureTemplate();
                        CreatureBaseStats const* stats = sObjectMgr->GetCreatureBaseStats(petlevel, cinfo->unit_class);
                        SetCreateHealth(m_owner->CountPctFromMaxHealth(40));
                        SetCreateMana(stats->BaseMana);

                        SetStat(STAT_STRENGTH, 22);
                        SetStat(STAT_AGILITY, 22);
                        SetStat(STAT_STAMINA, 25);
                        SetStat(STAT_INTELLECT, 28);
                        SetStat(STAT_SPIRIT, 27);
                    }
                    if (m_owner->GetTypeId() == TYPEID_PLAYER) // doomguard get 100% of owners spell, doombolt has his own coef.
                        SetSpellBonusDamage(int32(m_owner->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_SPELL)));
                    break;
                }
                case 50675: // ebon imp spell scaling
                    SetSpellBonusDamage(int32(m_owner->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_FIRE)));
                    break;
                case 24207:
                    SetCreateHealth(m_owner->CountPctFromMaxHealth(50));
                    SetSpellBonusDamage(int32(GetOwner()->GetTotalAttackPowerValue(BASE_ATTACK) * 0.5f));
                    SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, GetCreatureTemplate()->mindmg);
                    SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, GetCreatureTemplate()->maxdmg);
                    break;
            }
            break;
        }
    }

    UpdateAllStats();
    //    RecalculateAllScaling();
    SetFullHealth();
    SetPower(POWER_MANA, GetMaxPower(POWER_MANA));
    return true;
}

bool Pet::HaveInDiet(ItemTemplate const* item) const
{
    if (!item->FoodType)
        return false;

    CreatureTemplate const* cInfo = GetCreatureTemplate();
    if (!cInfo)
        return false;

    CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(cInfo->family);
    if (!cFamily)
        return false;

    uint32 diet = cFamily->petFoodMask;
    uint32 FoodMask = 1 << (item->FoodType-1);
    return diet & FoodMask;
}

uint32 Pet::GetCurrentFoodBenefitLevel(uint32 itemlevel) const
{
    // -5 or greater food level
    if (getLevel() <= itemlevel + 5)                         //possible to feed level 60 pet with level 55 level food for full effect
        return 35000;
    // -10..-6
    else if (getLevel() <= itemlevel + 10)                   //pure guess, but sounds good
        return 17000;
    // -14..-11
    else if (getLevel() <= itemlevel + 14)                   //level 55 food gets green on 70, makes sense to me
        return 8000;
    // -15 or less
    else
        return 0;                                           //food too low level
}

void Pet::_LoadSpellCooldowns(PetData* t_pet)
{
    time_t curTime = time(NULL);

    PacketCooldowns cooldowns;
    WorldPacket data;

    for (PetSpellCooldown::iterator itr = t_pet->m_spellscooldown.begin(), next = t_pet->m_spellscooldown.begin(); itr != t_pet->m_spellscooldown.end(); itr = next)
    {
        ++next;
        PetSpellCooldownData& m_spellcooldown = itr->second;

        uint32 spell_id = itr->first;
        time_t db_time  = time_t(m_spellcooldown.time);


        if (!sSpellMgr->GetSpellInfo(spell_id))
        {
            TC_LOG_ERROR("entities.pet", "Pet %u have unknown spell %u in `pet_spell_cooldown`, skipping.", m_charmInfo->GetPetNumber(), spell_id);
            m_spellcooldown._state = DATA_REMOVED;
            continue;
        }

        // skip outdated cooldown
        if (db_time <= curTime)
        {
            m_spellcooldown._state = DATA_REMOVED;
            continue;
        }

        cooldowns[spell_id] = uint32(db_time - curTime)*IN_MILLISECONDS;

        AddSpellCooldown(spell_id, 0, db_time);

        TC_LOG_DEBUG("entities.pet", "Pet (Number: %u) spell %u cooldown loaded (%u secs).", m_charmInfo->GetPetNumber(), spell_id, uint32(db_time-curTime));
    }

    if (!cooldowns.empty() && GetOwner())
    {
        BuildCooldownPacket(data, SPELL_COOLDOWN_FLAG_NONE, cooldowns);
        GetOwner()->GetSession()->SendPacket(&data);
    }
}

void Pet::_SaveSpellCooldowns(PetData* t_pet)
{
    t_pet->m_spellscooldown.clear();
    time_t curTime = time(NULL);

    // remove oudated and save active
    for (SpellCooldowns::iterator itr = m_spellCooldowns.begin(); itr != m_spellCooldowns.end();)
    {
        if (itr->second.end <= curTime)
            m_spellCooldowns.erase(itr++);
        else
        {
            PetSpellCooldownData t_data;
            t_data.time         = itr->second.end;
            t_data._state       = DATA_NEW;
            t_pet->m_spellscooldown[itr->first] = t_data;
            ++itr;
        }
    }
}

void Pet::_LoadSpells(PetData* t_pet)
{
    for (std::list<PetSpellData>::iterator itr = t_pet->m_learnSpells.begin(), next = t_pet->m_learnSpells.begin(); itr != t_pet->m_learnSpells.end(); itr = next)
    {
        ++next;
        PetSpellData& m_spell = (*itr);
        addSpell(m_spell.Id, m_spell);
    }
}

void Pet::_SaveSpells(PetData* t_pet)
{
    t_pet->m_learnSpells.clear();
    for (PetSpellMap::iterator itr = m_spells.begin(), next = m_spells.begin(); itr != m_spells.end(); itr = next)
    {
        ++next;

        // prevent saving family passives to DB
        if (itr->second.type == PETSPELL_FAMILY)
            continue;

        switch (itr->second._state)
        {
            case DATA_REMOVED:
                continue;
            default:
                t_pet->m_learnSpells.push_back(itr->second);
        }
        itr->second._state = DATA_UNCHANGED;
    }
}

void Pet::_LoadAuras(PetData* t_pet, uint32 timediff)
{
    _isAurasLoading = true;
    TC_LOG_DEBUG("entities.pet", "Loading auras for pet %u", GetGUIDLow());
    {
        ACE_Guard<ACE_Thread_Mutex> g(t_pet->m_auras.petAuraMapMutex);
        for (std::vector<PetAuraData>::iterator itr = t_pet->m_auras.auraList.begin(); itr != t_pet->m_auras.auraList.end(); ++itr)
        {
            PetAuraData& t_aura = *itr;

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(t_aura.spellid);
            if (!spellInfo)
            {
                TC_LOG_ERROR("entities.pet", "Unknown aura (spellid %u), ignore.", t_aura.spellid);
                t_aura._state = DATA_REMOVED;
                continue;
            }

            bool isAura = false;
            for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                if (spellInfo->Effects[i].IsAura())
                    isAura = true;

            if (!isAura)
            {
                t_aura._state = DATA_REMOVED;
                continue;
            }

            // negative effects should continue counting down after logout
            if (t_aura.remaintime != -1)
            {
                if (t_aura.remaintime/IN_MILLISECONDS <= int32(timediff))
                {
                    t_aura._state = DATA_REMOVED;
                    continue;
                }
                t_aura.remaintime -= timediff*IN_MILLISECONDS;
            }
            else if (t_aura.spellid == 53403) // Rabid Power exploit fix
            {
                t_aura._state = DATA_REMOVED;
                continue;
            }

            // prevent wrong values of remaincharges
            if (spellInfo->ProcCharges)
            {
                if (t_aura.remaincharges <= 0 || t_aura.remaincharges > spellInfo->ProcCharges)
                    t_aura.remaincharges = spellInfo->ProcCharges;
            }
            else
                t_aura.remaincharges = 0;

            // NULL guid stored - pet is the caster of the spell - see Pet::_SaveAuras
            if (!t_aura.caster_guid)
                t_aura.caster_guid  = GetGUID();

            if (Aura* aura = Aura::TryCreate(spellInfo, t_aura.effmask, this, NULL, &t_aura.baseDamage[0], NULL, t_aura.caster_guid))
            {
                if (!aura->CanBeSaved())
                {
                    aura->Remove();
                    t_aura._state = DATA_REMOVED;
                    continue;
                }
                aura->SetLoadedState(t_aura.maxduration, t_aura.remaintime, t_aura.remaincharges, t_aura.stackcount, t_aura.recalculatemask, &t_aura.damage[0]);
                aura->ApplyForTargets();
                TC_LOG_INFO("entities.pet", "Added aura spellid %u, effectmask %u", spellInfo->Id, t_aura.effmask);
            }
        }
    }
    _isAurasLoading = false;
    _SaveAuras(t_pet);
}

void Pet::_SaveAuras(PetData* t_pet)
{
    if (_isAurasLoading)
        return;
    ACE_Guard<ACE_Thread_Mutex> g(t_pet->m_auras.petAuraMapMutex);
    t_pet->m_auras.auraList.clear();

    for (AuraMap::const_iterator itr = m_ownedAuras.begin(); itr != m_ownedAuras.end(); ++itr)
    {
        // check if the aura has to be saved
        if (!itr->second->CanBeSaved() || IsPetAura(itr->second))
            continue;

        Aura* aura = itr->second;

        PetAuraData t_aura;

        uint8 effMask = 0;
        uint8 recalculateMask = 0;
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
        {
            if (aura->GetEffect(i))
            {
                t_aura.baseDamage[i] = aura->GetEffect(i)->GetBaseAmount();
                t_aura.damage[i] = aura->GetEffect(i)->GetAmount();
                effMask |= (1<<i);
                if (aura->GetEffect(i)->CanBeRecalculated())
                    recalculateMask |= (1<<i);
            }
            else
            {
                t_aura.baseDamage[i] = 0;
                t_aura.damage[i] = 0;
            }
        }

        // don't save guid of caster in case we are caster of the spell - guid for pet is generated every pet load, so it won't match saved guid anyways
        t_aura.caster_guid      = (itr->second->GetCasterGUID() == GetGUID()) ? 0 : itr->second->GetCasterGUID();
        t_aura.spellid          = itr->second->GetId();
        t_aura.maxduration      = itr->second->GetMaxDuration();
        t_aura.remaincharges    = itr->second->GetCharges();
        t_aura.stackcount       = itr->second->GetStackAmount();
        t_aura.remaintime       = itr->second->GetDuration();
        t_aura.effmask          = effMask;
        t_aura.recalculatemask  = recalculateMask;
        t_aura._state           = DATA_NEW;
        t_pet->m_auras.auraList.push_back(t_aura);
    }
}

bool Pet::addSpell(uint32 spellId,PetSpellData& m_spell)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
    {
        TC_LOG_ERROR("entities.pet", "Pet::addSpell: Non-existed in SpellStore spell #%u request.", spellId);
        m_spell._state = DATA_REMOVED;
        return false;
    }

    PetSpellMap::iterator itr = m_spells.find(spellId);
    if (itr != m_spells.end())
    {
        if (itr->second._state == DATA_REMOVED)
        {
            m_spells.erase(itr);
            m_spell._state = DATA_CHANGED;
        }
        else if (m_spell._state == DATA_UNCHANGED && itr->second._state != DATA_UNCHANGED)
        {
            // can be in case spell loading but learned at some previous spell loading
            itr->second._state = DATA_UNCHANGED;

            if (m_spell.active == ACT_ENABLED)
                ToggleAutocast(spellInfo, true);
            else if (m_spell.active == ACT_DISABLED)
                ToggleAutocast(spellInfo, false);

            return false;
        }
        else
            return false;
    }

    if (m_spell.active == ACT_DECIDE)                               // active was not used before, so we save it's autocast/passive state here
    {
        if (spellInfo->IsAutocastable())
            m_spell.active = ACT_DISABLED;
        else
            m_spell.active = ACT_PASSIVE;
    }

    // talent: unlearn all other talent ranks (high and low)
    if (TalentSpellPos const* talentPos = GetTalentSpellPos(spellId))
    {
        if (TalentEntry const* talentInfo = sTalentStore.LookupEntry(talentPos->talent_id))
        {
            for (uint8 i = 0; i < MAX_TALENT_RANK; ++i)
            {
                // skip learning spell and no rank spell case
                uint32 rankSpellId = talentInfo->RankID[i];
                if (!rankSpellId || rankSpellId == spellId)
                    continue;

                // skip unknown ranks
                if (!HasSpell(rankSpellId))
                    continue;
                removeSpell(rankSpellId, false, false);
            }
        }
    }
    else if (spellInfo->IsRanked())
    {
        for (PetSpellMap::const_iterator itr2 = m_spells.begin(); itr2 != m_spells.end(); ++itr2)
        {
            if (itr2->second._state == DATA_REMOVED)
                continue;

            SpellInfo const* oldRankSpellInfo = sSpellMgr->GetSpellInfo(itr2->first);

            if (!oldRankSpellInfo)
                continue;

            if (spellInfo->IsDifferentRankOf(oldRankSpellInfo))
            {
                // replace by new high rank
                if (spellInfo->IsHighRankOf(oldRankSpellInfo))
                {
                    m_spell.active = itr2->second.active;

                    if (m_spell.active == ACT_ENABLED)
                        ToggleAutocast(oldRankSpellInfo, false);

                    unlearnSpell(itr2->first, false, false);
                    break;
                }
                // ignore new lesser rank
                else
                    return false;
            }
        }
    }

    m_spells[spellId] = m_spell;

    if (spellInfo->IsPassive() && (!spellInfo->CasterAuraState || HasAuraState(AuraStateType(spellInfo->CasterAuraState))))
        CastSpell(this, spellId, true);
    else
        m_charmInfo->AddSpellToActionBar(spellInfo);

    if (m_spell.active == ACT_ENABLED)
        ToggleAutocast(spellInfo, true);

    uint32 talentCost = GetTalentSpellCost(spellId);
    if (talentCost)
    {
        int32 free_points = GetMaxTalentPointsForLevel(getLevel());
        m_usedTalentCount += talentCost;
        // update free talent points
        free_points-=m_usedTalentCount;
        SetFreeTalentPoints(free_points > 0 ? free_points : 0);
    }
    return true;
}

bool Pet::learnSpell(uint32 spell_id)
{
    // prevent duplicated entires in spell book
    PetSpellData m_spell;
    m_spell.active  = ACT_DECIDE;
    m_spell.type    = PETSPELL_NORMAL;
    m_spell._state  = DATA_NEW;
    m_spell.Id      = spell_id;

    if (!addSpell(spell_id,m_spell))
        return false;

    if (!m_loading)
    {
        WorldPacket data(SMSG_PET_LEARNED_SPELL, 4);
        data << uint32(spell_id);
        m_owner->GetSession()->SendPacket(&data);
        m_owner->PetSpellInitialize();
    }
    return true;
}

void Pet::InitLevelupSpellsForLevel()
{
    uint8 level = getLevel();

    if (PetLevelupSpellSet const* levelupSpells = GetCreatureTemplate()->family ? sSpellMgr->GetPetLevelupSpellList(GetCreatureTemplate()->family) : NULL)
    {
        // PetLevelupSpellSet ordered by levels, process in reversed order
        for (PetLevelupSpellSet::const_reverse_iterator itr = levelupSpells->rbegin(); itr != levelupSpells->rend(); ++itr)
        {
            // will called first if level down
            if (itr->first > level)
                unlearnSpell(itr->second, true);                 // will learn prev rank if any
            // will called if level up
            else
                learnSpell(itr->second);                        // will unlearn prev rank if any
        }
    }

    int32 petSpellsId = GetCreatureTemplate()->PetSpellDataId ? -(int32)GetCreatureTemplate()->PetSpellDataId : GetEntry();

    // default spells (can be not learned if pet level (as owner level decrease result for example) less first possible in normal game)
    if (PetDefaultSpellsEntry const* defSpells = sSpellMgr->GetPetDefaultSpellsEntry(petSpellsId))
    {
        for (uint8 i = 0; i < MAX_CREATURE_SPELL_DATA_SLOT; ++i)
        {
            SpellInfo const* spellEntry = sSpellMgr->GetSpellInfo(defSpells->spellid[i]);
            if (!spellEntry)
                continue;

            // will called first if level down
            if (spellEntry->SpellLevel > level)
                unlearnSpell(spellEntry->Id, true);
            // will called if level up
            else
                learnSpell(spellEntry->Id);
        }
    }
}

bool Pet::unlearnSpell(uint32 spell_id, bool learn_prev, bool clear_ab)
{
    if (removeSpell(spell_id, learn_prev, clear_ab))
    {
        if (!m_loading)
        {
            WorldPacket data(SMSG_PET_REMOVED_SPELL, 4);
            data << uint32(spell_id);
            m_owner->GetSession()->SendPacket(&data);
        }
        return true;
    }
    return false;
}

bool Pet::removeSpell(uint32 spell_id, bool learn_prev, bool clear_ab)
{
    PetSpellMap::iterator itr = m_spells.find(spell_id);
    if (itr == m_spells.end())
        return false;

    if (itr->second._state == DATA_REMOVED)
        return false;

    if (itr->second._state == DATA_NEW)
        m_spells.erase(itr);
    else
        itr->second._state = DATA_REMOVED;

    RemoveAurasDueToSpell(spell_id);

    uint32 talentCost = GetTalentSpellCost(spell_id);
    if (talentCost > 0)
    {
        if (m_usedTalentCount > talentCost)
            m_usedTalentCount -= talentCost;
        else
            m_usedTalentCount = 0;
        // update free talent points
        int32 free_points = GetMaxTalentPointsForLevel(getLevel()) - m_usedTalentCount;
        SetFreeTalentPoints(free_points > 0 ? free_points : 0);
    }

    if (learn_prev)
    {
        if (uint32 prev_id = sSpellMgr->GetPrevSpellInChain (spell_id))
            learnSpell(prev_id);
        else
            learn_prev = false;
    }

    // if remove last rank or non-ranked then update action bar at server and client if need
    if (clear_ab && !learn_prev && m_charmInfo->RemoveSpellFromActionBar(spell_id))
    {
        if (!m_loading)
        {
            // need update action bar for last removed rank
            if (Unit* owner = GetOwner())
                if (owner->GetTypeId() == TYPEID_PLAYER)
                    owner->ToPlayer()->PetSpellInitialize();
        }
    }

    return true;
}

void Pet::CleanupActionBar()
{
    for (uint8 i = 0; i < MAX_UNIT_ACTION_BAR_INDEX; ++i)
        if (UnitActionBarEntry const* ab = m_charmInfo->GetActionBarEntry(i))
            if (ab->GetAction() && ab->IsActionBarForSpell())
            {
                if (!HasSpell(ab->GetAction()))
                    m_charmInfo->SetActionBar(i, 0, ACT_PASSIVE);
                else if (ab->GetType() == ACT_ENABLED)
                {
                    if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(ab->GetAction()))
                        ToggleAutocast(spellInfo, true);
                }
            }
}

void Pet::InitPetCreateSpells()
{
    m_charmInfo->InitPetActionBar();
    m_spells.clear();

    LearnPetPassives();
    InitLevelupSpellsForLevel();

    CastPetAuras(false);
}

bool Pet::resetTalents()
{
    Unit* owner = GetOwner();
    if (!owner || owner->GetTypeId() != TYPEID_PLAYER)
        return false;

    // not need after this call
    if (owner->ToPlayer()->HasAtLoginFlag(AT_LOGIN_RESET_PET_TALENTS))
        owner->ToPlayer()->RemoveAtLoginFlag(AT_LOGIN_RESET_PET_TALENTS, true);

    CreatureTemplate const* ci = GetCreatureTemplate();
    if (!ci)
        return false;
    // Check pet talent type
    CreatureFamilyEntry const* pet_family = sCreatureFamilyStore.LookupEntry(ci->family);
    if (!pet_family || pet_family->petTalentType < 0)
        return false;

    Player* player = owner->ToPlayer();

    uint8 level = getLevel();
    uint32 talentPointsForLevel = GetMaxTalentPointsForLevel(level);

    if (m_usedTalentCount == 0)
    {
        SetFreeTalentPoints(talentPointsForLevel);
        return false;
    }

    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const* talentInfo = sTalentStore.LookupEntry(i);

        if (!talentInfo)
            continue;

        TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);

        if (!talentTabInfo)
            continue;

        // unlearn only talents for pets family talent type
        if (!((1 << pet_family->petTalentType) & talentTabInfo->petTalentMask))
            continue;

        for (uint8 j = 0; j < MAX_TALENT_RANK; ++j)
        {
            for (PetSpellMap::const_iterator itr = m_spells.begin(); itr != m_spells.end();)
            {
                if (itr->second._state == DATA_REMOVED)
                {
                    ++itr;
                    continue;
                }
                // remove learned spells (all ranks)
                uint32 itrFirstId = sSpellMgr->GetFirstSpellInChain(itr->first);

                // unlearn if first rank is talent or learned by talent
                if (itrFirstId == talentInfo->RankID[j] || sSpellMgr->IsSpellLearnToSpell(talentInfo->RankID[j], itrFirstId))
                {
                    unlearnSpell(itr->first, false);
                    itr = m_spells.begin();
                    continue;
                }
                else
                    ++itr;
            }
        }
    }

    SetFreeTalentPoints(talentPointsForLevel);

    if (!m_loading)
        player->PetSpellInitialize();
    return true;
}

void Pet::resetTalentsForAllPetsOf(Player* owner, Pet* online_pet /*= NULL*/)
{
    // not need after this call
    if (owner->ToPlayer()->HasAtLoginFlag(AT_LOGIN_RESET_PET_TALENTS))
        owner->ToPlayer()->RemoveAtLoginFlag(AT_LOGIN_RESET_PET_TALENTS, true);

    // reset for online
    if (online_pet)
        online_pet->resetTalents();
}

void Pet::InitTalentForLevel()
{
    uint8 level = getLevel();
    uint32 talentPointsForLevel = GetMaxTalentPointsForLevel(level);
    // Reset talents in case low level (on level down) or wrong points for level (hunter can unlearn TP increase talent)
    if (talentPointsForLevel == 0 || m_usedTalentCount > talentPointsForLevel)
        resetTalents(); // Remove all talent points

    SetFreeTalentPoints(talentPointsForLevel - m_usedTalentCount);

    Unit* owner = GetOwner();
    if (!owner || owner->GetTypeId() != TYPEID_PLAYER)
        return;

    if (!m_loading)
        owner->ToPlayer()->SendTalentsInfoData(true);
}

uint8 Pet::GetMaxTalentPointsForLevel(uint8 level)
{
    uint8 points = (level >= 20) ? ((level - 16) / 4) : 0;
    // Mod points from owner SPELL_AURA_MOD_PET_TALENT_POINTS
    if (Unit* owner = GetOwner())
        points+=owner->GetTotalAuraModifier(SPELL_AURA_MOD_PET_TALENT_POINTS);
    return points;
}

void Pet::ToggleAutocast(SpellInfo const* spellInfo, bool apply)
{
    if (!spellInfo->IsAutocastable())
        return;

    uint32 spellid = spellInfo->Id;

    PetSpellMap::iterator itr = m_spells.find(spellid);
    if (itr == m_spells.end())
        return;

    uint32 i;

    if (apply)
    {
        for (i = 0; i < m_autospells.size() && m_autospells[i] != spellid; ++i)
            ;                                               // just search

        if (i == m_autospells.size())
        {
            m_autospells.push_back(spellid);

            if (itr->second.active != ACT_ENABLED)
            {
                itr->second.active = ACT_ENABLED;
                if (itr->second._state != DATA_NEW)
                    itr->second._state = DATA_CHANGED;
            }
        }
    }
    else
    {
        AutoSpellList::iterator itr2 = m_autospells.begin();
        for (i = 0; i < m_autospells.size() && m_autospells[i] != spellid; ++i, ++itr2)
            ;                                               // just search

        if (i < m_autospells.size())
        {
            m_autospells.erase(itr2);
            if (itr->second.active != ACT_DISABLED)
            {
                itr->second.active = ACT_DISABLED;
                if (itr->second._state != DATA_NEW)
                    itr->second._state = DATA_CHANGED;
            }
        }
    }
}

bool Pet::IsPermanentPetFor(Player* owner) const
{
    switch (getPetType())
    {
        case SUMMON_PET:
            switch (owner->getClass())
            {
                case CLASS_WARLOCK:
                    return GetCreatureTemplate()->type == CREATURE_TYPE_DEMON;
                case CLASS_DEATH_KNIGHT:
                    return GetCreatureTemplate()->type == CREATURE_TYPE_UNDEAD;
                case CLASS_MAGE:
                    return GetCreatureTemplate()->type == CREATURE_TYPE_ELEMENTAL;
                default:
                    return false;
            }
        case HUNTER_PET:
            return true;
        default:
            return false;
    }
}

bool Pet::Create(uint32 guidlow, Map* map, uint32 phaseMask, uint32 Entry, uint32 pet_number)
{
    ASSERT(map);
    SetMap(map);

    SetPhaseMask(phaseMask, false);
    Object::_Create(guidlow, pet_number, HIGHGUID_PET);

    m_DBTableGuid = guidlow;
    m_originalEntry = Entry;

    if (!InitEntry(Entry))
        return false;

    // Force regen flag for player pets, just like we do for players themselves
    SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_REGENERATE_POWER);
    SetSheath(SHEATH_STATE_MELEE);

    return true;
}

bool Pet::HasSpell(uint32 spell) const
{
    PetSpellMap::const_iterator itr = m_spells.find(spell);
    return itr != m_spells.end() && itr->second._state != DATA_REMOVED;
}

// Get all passive spells in our skill line
void Pet::LearnPetPassives()
{
    CreatureTemplate const* cInfo = GetCreatureTemplate();
    if (!cInfo)
        return;

    CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(cInfo->family);
    if (!cFamily)
        return;

    PetFamilySpellsStore::const_iterator petStore = sPetFamilySpellsStore.find(cFamily->ID);
    if (petStore != sPetFamilySpellsStore.end())
    {
        // For general hunter pets skill 270
        // Passive 01~10, Passive 00 (20782, not used), Ferocious Inspiration (34457)
        // Scale 01~03 (34902~34904, bonus from owner, not used)
        for (PetFamilySpellsSet::const_iterator petSet = petStore->second.begin(); petSet != petStore->second.end(); ++petSet)
        {
            PetSpellData m_spell;
            m_spell.active  = ACT_DECIDE;
            m_spell.type    = PETSPELL_FAMILY;
            m_spell._state  = DATA_NEW;
            m_spell.Id      = *petSet;
            addSpell(*petSet, m_spell);
        }
    }
}

void Pet::CastPetAuras(bool current)
{
    Unit* owner = GetOwner();
    if (!owner || owner->GetTypeId() != TYPEID_PLAYER)
        return;

    if (!IsPermanentPetFor(owner->ToPlayer()))
        return;

    for (PetAuraSet::const_iterator itr = owner->m_petAuras.begin(); itr != owner->m_petAuras.end();)
    {
        PetAura const* pa = *itr;
        ++itr;

        if (!current && pa->IsRemovedOnChangePet())
            owner->RemovePetAura(pa);
        else
            CastPetAura(pa);
    }

    if (IsPetGhoul())
    {
        RemoveAurasDueToSpell(61697);
        CastSpell(this, 61697, true);
        RemoveAurasDueToSpell(62137);
        CastSpell(this, 62137, true);
    }

    if (isHunterPet())
    {
        RemoveAurasDueToSpell(65220);
        CastSpell(this, 65220, true);
    }
}

void Pet::CastPetAura(PetAura const* aura)
{
    uint32 auraId = aura->GetAura(GetEntry());
    if (!auraId)
        return;

    if (auraId == 35696)                                      // Demonic Knowledge
    {
        int32 basePoints = CalculatePct(aura->GetDamage(), GetStat(STAT_STAMINA) + GetStat(STAT_INTELLECT));
        CastCustomSpell(this, auraId, &basePoints, NULL, NULL, true);
    }
    else
        CastSpell(this, auraId, true);
}

bool Pet::IsPetAura(Aura const* aura)
{
    Unit* owner = GetOwner();

    if (!owner || owner->GetTypeId() != TYPEID_PLAYER)
        return false;

    // if the owner has that pet aura, return true
    for (PetAuraSet::const_iterator itr = owner->m_petAuras.begin(); itr != owner->m_petAuras.end(); ++itr)
    {
        if ((*itr)->GetAura(GetEntry()) == aura->GetId())
            return true;
    }
    return false;
}

void Pet::learnSpellHighRank(uint32 spellid)
{
    learnSpell(spellid);

    if (uint32 next = sSpellMgr->GetNextSpellInChain(spellid))
        learnSpellHighRank(next);
}

void Pet::SynchronizeLevelWithOwner()
{
    Unit* owner = GetOwner();
    if (!owner || owner->GetTypeId() != TYPEID_PLAYER)
        return;

    switch (getPetType())
    {
        // always same level
        case SUMMON_PET:
        case HUNTER_PET:
            GivePetLevel(owner->getLevel());
            break;
        default:
            break;
    }
}

void Pet::ProhibitSpellSchool(SpellSchoolMask idSchoolMask, uint32 unTimeMs)
{
    PacketCooldowns cooldowns;
    WorldPacket data;
    time_t curTime = time(NULL);
    for (PetSpellMap::const_iterator itr = m_spells.begin(); itr != m_spells.end(); ++itr)
    {
        if (itr->second._state == DATA_REMOVED)
            continue;
        uint32 unSpellId = itr->first;
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(unSpellId);
        ASSERT(spellInfo);

        // Not send cooldown for this spells
        if (spellInfo->Attributes & SPELL_ATTR0_DISABLED_WHILE_ACTIVE)
            continue;

        if (spellInfo->PreventionType != SPELL_PREVENTION_TYPE_SILENCE)
            continue;

        if ((idSchoolMask & spellInfo->GetSchoolMask()) && GetSpellCooldownDelay(unSpellId) < unTimeMs)
        {
            cooldowns[unSpellId] = unTimeMs;
            AddSpellCooldown(unSpellId, 0, curTime + unTimeMs/IN_MILLISECONDS);
        }
    }

    if (!cooldowns.empty())
    {
        BuildCooldownPacket(data, SPELL_COOLDOWN_FLAG_NONE, cooldowns);

        if (Player* owner = GetOwner())
            owner->GetSession()->SendPacket(&data);
    }
}

void Guardian::RecalculateAllScaling()
{
    for (unsigned int i = 0; i <= SPELL_SCHOOL_ARCANE; i++)
        RecalculatePetsScalingResistance((SpellSchools)i);
    for (unsigned int i = 0; i < MAX_STATS; i++)
        RecalculatePetScalingStats((Stats)i);
    RecalculatePetScalingAttackPower();
    RecalculatePetScalingDamageDone();
    RecalculatePetScalingDamageDonePct();
    for (unsigned int cnt = 0; cnt < MAX_ATTACK; cnt++)
        RecalculatePetScalingAttackSpeed((WeaponAttackType)cnt);
    RecalculatePetScalingHitRating();
    RecalculatePetScalingCritRating();
}

void Guardian::RecalculatePetScalingResistance(uint32 school)
{
    uint32 auraId = 0;
    uint8 effIndex = EFFECT_0;

    switch (school)
    {
        case SPELL_SCHOOL_NORMAL:
            if (isHunterPet())
            {
                auraId = 34904;
                effIndex = EFFECT_2;
            }
            else if (m_owner && m_owner->getClass() == CLASS_WARLOCK)
            {
                auraId = 34956;
                effIndex = EFFECT_1;
            }
            break;
        case SPELL_SCHOOL_FIRE:
            if (isHunterPet())
            {
                auraId = 34903;
                effIndex = EFFECT_0;
            }
            else if (m_owner && m_owner->getClass() == CLASS_WARLOCK)
            {
                auraId = 34956;
                effIndex = EFFECT_2;
            }
            break;
        case SPELL_SCHOOL_NATURE:
            if (isHunterPet())
            {
                auraId = 34903;
                effIndex = EFFECT_2;
            }
            else if (m_owner && m_owner->getClass() == CLASS_WARLOCK)
            {
                auraId = 34957;
                effIndex = EFFECT_2;
            }
            break;
        case SPELL_SCHOOL_FROST:
            if (isHunterPet())
            {
                auraId = 34903;
                effIndex = EFFECT_1;
            }
            else if (m_owner && m_owner->getClass() == CLASS_WARLOCK)
            {
                auraId = 34957;
                effIndex = EFFECT_0;
            }
            break;
        case SPELL_SCHOOL_SHADOW:
            if (isHunterPet())
            {
                auraId = 34904;
                effIndex = EFFECT_0;
            }
            else if (m_owner && m_owner->getClass() == CLASS_WARLOCK)
            {
                auraId = 34958;
                effIndex = EFFECT_0;
            }
            break;
        case SPELL_SCHOOL_ARCANE:
            if (isHunterPet())
            {
                auraId = 34904;
                effIndex = EFFECT_1;
            }
            else if (m_owner && m_owner->getClass() == CLASS_WARLOCK)
            {
                auraId = 34957;
                effIndex = EFFECT_1;
            }
            break;
    }

    if (!auraId)
        return;

    if (AuraEffect* aurEffect = GetAuraEffect(auraId, effIndex))
        aurEffect->RecalculateAmount();
}

void Guardian::RecalculatePetScalingStats(Stats stat)
{
    if (stat >= MAX_STATS)
        return;

    uint32 auraId = 0;
    uint8 effIndex = EFFECT_0;

    switch (stat)
    {
        case STAT_STAMINA:
            if (IsPetGhoul())
            {
                auraId = 54566;
                effIndex = EFFECT_0;
            }
            else if (isHunterPet())
            {
                auraId = 34902;
                effIndex = EFFECT_0;
            }
            else if (m_owner && m_owner->getClass() == CLASS_WARLOCK)
            {
                auraId = 34947;
                effIndex = EFFECT_0;
            }
            break;
        case STAT_STRENGTH:
            if (IsPetGhoul())
            {
                auraId = 54566;
                effIndex = EFFECT_1;
            }
            break;
        case STAT_INTELLECT:
            if (m_owner && m_owner->getClass() == CLASS_WARLOCK)
            {
                auraId = 34956;
                effIndex = EFFECT_0;
            }
            break;
    }

    if (!auraId)
        return;

    if (AuraEffect* aurEffect = GetAuraEffect(auraId, effIndex))
        aurEffect->RecalculateAmount();
}

void Guardian::RecalculatePetScalingAttackPower()
{
    uint32 auraId = 0;
    uint8 effIndex = EFFECT_0;

    if (isHunterPet())
    {
        auraId = 34902;
        effIndex = EFFECT_1;
    }
    else if (IsPetGhoul())
    {
        auraId = 0;
        effIndex = EFFECT_0;
    }
    else if (m_owner && m_owner->getClass() == CLASS_WARLOCK)
    {
        auraId = 34947;
        effIndex = EFFECT_1;
    }

    if (!auraId)
        return;

    if (AuraEffect* aurEffect = GetAuraEffect(auraId, effIndex))
        aurEffect->RecalculateAmount();
}

void Guardian::RecalculatePetScalingDamageDone()
{
    uint32 auraId = 0;
    uint8 effIndex = EFFECT_0;

    if (isHunterPet())
    {
        auraId = 34902;
        effIndex = EFFECT_2;
    }
    else if (IsPetGhoul())
    {
        auraId = 54566;
        effIndex = EFFECT_2;
    }
    else if (GetEntry() == 27893)
    {
        auraId = 51906;
        effIndex = EFFECT_0;
    }
    else if (m_owner && m_owner->getClass() == CLASS_WARLOCK)
    {
        auraId = 34947;
        effIndex = EFFECT_2;
    }

    if (!auraId)
        return;

    if (AuraEffect* aurEffect = GetAuraEffect(auraId, effIndex))
        aurEffect->RecalculateAmount();
}

void Guardian::RecalculatePetScalingDamageDonePct()
{
    uint32 auraId = 0;
    uint8 effIndex = EFFECT_0;

    if (IsPetGhoul())
    {
        auraId = 51996;
        effIndex = EFFECT_0;
    }

    if (!auraId)
        return;

    if (AuraEffect* aurEffect = GetAuraEffect(auraId, effIndex))
        aurEffect->RecalculateAmount();
}

void Guardian::RecalculatePetScalingAttackSpeed(WeaponAttackType /*att*/)
{
    uint32 auraId = 0;
    uint8 effIndex = EFFECT_1;
    if (isHunterPet())
        auraId = 89446;
    else if (IsPetGhoul())
        auraId = 51996;
    else if (GetEntry() == 27893)
        auraId = 51906;
    else if (GetEntry() == 510) // water elemental
        auraId = 89764;

    if (!auraId)
        return;

    if (AuraEffect* aurEffect = GetAuraEffect(auraId, effIndex))
        aurEffect->RecalculateAmount();
}

void Guardian::RecalculatePetScalingHitRating()
{
    uint32 auraId = 0;

    if (isHunterPet())
    {
        auraId = 61017;
    }
    else if (IsPetGhoul() || GetEntry() == 27829)
    {
        auraId = 61697;
    }
    else if (GetEntry() == 29264)
    {
        auraId = 61783;
    }
    else  if (m_owner && m_owner->getClass() == CLASS_WARLOCK)
    {
        auraId = 61013;
    }

    if (!auraId)
        return;

    if (Aura* aura = GetAura(auraId))
        aura->RecalculateAmountOfEffects();
}

void Guardian::RecalculatePetScalingCritRating()
{
    uint32 auraId = 0;
    uint8 effIndex = EFFECT_0;

    if (isHunterPet())
    {
        auraId = 89446;
        effIndex = EFFECT_0;
    }
    else if (m_owner && m_owner->getClass() == CLASS_WARLOCK)
    {
        auraId = 35695;
        effIndex = EFFECT_1;
    }
    else if (IsPetGhoul())
    {
        auraId = 110474;
        effIndex = EFFECT_0;
    }
    else if (GetEntry() == 510) // water elemental
    {
        auraId = 89764;
        effIndex = EFFECT_0;
    }
    if (!auraId)
        return;

    if (Aura* aura = GetAura(auraId))
        aura->RecalculateAmountOfEffects();
 }

void Guardian::RecalculateHappinessEffect()
{
    uint32 auraId = 0;
    uint8 effIndex = EFFECT_0;

    if (isHunterPet())
    {
        auraId = 8875;
        effIndex = EFFECT_0;
    }

    if (!auraId)
        return;

    if (AuraEffect* aurEffect = GetAuraEffect(auraId, effIndex))
        aurEffect->RecalculateAmount();
}

void Pet::SetDisplayId(uint32 modelId)
{
    Guardian::SetDisplayId(modelId);

    if (!isControlled())
        return;

    if (Unit* owner = GetOwner())
        if (Player* player = owner->ToPlayer())
            if (player->GetGroup())
                player->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_MODEL_ID);
}
