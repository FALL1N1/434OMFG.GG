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

#include "TotemAI.h"
#include "Totem.h"
#include "Creature.h"
#include "DBCStores.h"
#include "ObjectAccessor.h"
#include "SpellMgr.h"

#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"

int TotemAI::Permissible(Creature const* creature)
{
    if (creature->isTotem())
        return PERMIT_BASE_PROACTIVE;

    return PERMIT_BASE_NO;
}

TotemAI::TotemAI(Creature* c) : CreatureAI(c), i_victimGuid(0)
{
    ASSERT(c->isTotem());
}

void TotemAI::MoveInLineOfSight(Unit* /*who*/)
{
}

void TotemAI::EnterEvadeMode()
{
    me->CombatStop(true);
}

void TotemAI::UpdateAI(uint32 const /*diff*/)
{
    if (me->ToTotem()->GetTotemType() != TOTEM_ACTIVE)
        return;

    if (!me->isAlive() || me->IsNonMeleeSpellCasted(false))
        return;

    // Search spell
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(me->ToTotem()->GetSpell());
    if (!spellInfo)
        return;

    // Get spell range
    float max_range = spellInfo->GetMaxRange(false);

    // SPELLMOD_RANGE not applied in this place just because not existence range mods for attacking totems

    // In the case of the totem searing, first try a target that has the dot of flame shock
    // Otherwise, look for the nearest target

    if (me->GetEntry() == 2523)
    {
        if (!me->GetOwner())
            return;

        // Totemic Reach
        if (AuraEffect * aurEff = me->GetOwner()->GetAuraEffect(SPELL_AURA_ADD_PCT_MODIFIER, SPELLFAMILY_SHAMAN, 334, EFFECT_0))
        {
            AddPct(max_range, aurEff->GetAmount());
        }

        // Elemental Reach
        if (AuraEffect * aurEff = me->GetOwner()->GetAuraEffect(SPELL_AURA_ADD_FLAT_MODIFIER, SPELLFAMILY_SHAMAN, 1921, EFFECT_2))
        {
            max_range += aurEff->GetAmount();
        }

        std::list<Unit*> targets;
        Trinity::AnyUnfriendlyUnitInObjectRangeCheck u_check(me, me, max_range);
        Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(me, targets, u_check);
        me->VisitNearbyObject(max_range, searcher);

        for (std::list<Unit*>::const_iterator iter = targets.begin(); iter != targets.end(); ++iter)
        {
            if ((*iter) && (*iter)->isTargetableForAttack()
                && me->IsWithinDistInMap((*iter), max_range)
                && !me->IsFriendlyTo((*iter))
                && me->canSeeOrDetect((*iter))
                && ((*iter)->HasAura(8050, me->ToTotem()->GetOwnerGUID()) || (*iter)->HasAura(17364, me->ToTotem()->GetOwnerGUID())))
            {
                i_victimGuid = (*iter)->GetGUID();
                break;
            }

            else if (!(*iter)->HasAura(8050, me->ToTotem()->GetOwnerGUID()) && !(*iter)->HasAura(17364, me->ToTotem()->GetOwnerGUID()))
            {
                Unit* victim = i_victimGuid ? ObjectAccessor::GetUnit(*me, i_victimGuid) : NULL;
                Trinity::NearestAttackableUnitInObjectRangeCheck u_check(me, me, max_range);
                Trinity::UnitLastSearcher<Trinity::NearestAttackableUnitInObjectRangeCheck> checker(me, victim, u_check);
                me->VisitNearbyObject(max_range, checker);

                if (victim)
                    i_victimGuid = victim->GetGUID();
            }
        }

        Unit* memtarget = i_victimGuid ? ObjectAccessor::GetUnit(*me, i_victimGuid) : NULL;

        if (memtarget && me->GetOwner()->isInCombat())
        {
            me->CastSpell(memtarget, me->ToTotem()->GetSpell(), false);
        }
    }
    else
    {
        // pointer to appropriate target if found any
        Unit* victim = i_victimGuid ? ObjectAccessor::GetUnit(*me, i_victimGuid) : NULL;

        // Search victim if no, not attackable, or out of range, or friendly (possible in case duel end)
        if (!victim ||
            !victim->isTargetableForAttack() || !me->IsWithinDistInMap(victim, max_range) ||
            me->IsFriendlyTo(victim) || !me->canSeeOrDetect(victim))
        {
            victim = NULL;
            Trinity::NearestAttackableUnitInObjectRangeCheck u_check(me, me, max_range);
            Trinity::UnitLastSearcher<Trinity::NearestAttackableUnitInObjectRangeCheck> checker(me, victim, u_check);
            me->VisitNearbyObject(max_range, checker);
        }

        // If have target
        if (victim)
        {
            // remember
            i_victimGuid = victim->GetGUID();

            // attack
            me->SetInFront(victim);                         // client change orientation by self
            me->CastSpell(victim, me->ToTotem()->GetSpell(), false);
        }
        else
            i_victimGuid = 0;
    }
}

void TotemAI::AttackStart(Unit* /*victim*/)
{
    // Sentry totem sends ping on attack
    if (me->GetEntry() == SENTRY_TOTEM_ENTRY && me->GetOwner()->GetTypeId() == TYPEID_PLAYER)
    {
        WorldPacket data(MSG_MINIMAP_PING, (8+4+4));
        data << me->GetGUID();
        data << me->GetPositionX();
        data << me->GetPositionY();
        ((Player*)me->GetOwner())->GetSession()->SendPacket(&data);
    }
}
