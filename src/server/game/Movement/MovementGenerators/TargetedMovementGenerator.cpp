/*
* Copyright (C) 2008-2017 TrinityCore <http://www.trinitycore.org/>
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

#include "ByteBuffer.h"
#include "TargetedMovementGenerator.h"
#include "Errors.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "World.h"
#include "MoveSplineInit.h"
#include "MoveSpline.h"
#include "Player.h"
#include "Pet.h"
#include "Spell.h"
#include "Transport.h"
#include "PathGenerator.h"
#include "VehicleDefines.h"

template<class T, typename D>
void TargetedMovementGeneratorMedium<T, D>::_setTargetLocation(T* owner, bool updateDestination)
{
    if (!i_target.isValid() || !i_target->IsInWorld())
        return;

    if (owner->HasUnitState(UNIT_STATE_NOT_MOVE))
        return;

	if (owner->HasUnitState(UNIT_STATE_CASTING) && !owner->CanMoveDuringChannel())
		return;

	bool forceDest = owner->GetTypeId() == TYPEID_UNIT && owner->HasUnitState(UNIT_STATE_FOLLOW);
	if (owner->GetTypeId() == TYPEID_UNIT && !forceDest && !i_target->isInAccessiblePlaceFor(owner->ToCreature()))
		return;

	if (owner->GetTypeId() == TYPEID_UNIT && owner->ToCreature()->IsFocusing(NULL, true))
		return;

    float x, y, z;

	Player* player = 0;
    if (updateDestination || !i_path)
    {
		if (!i_path)
			i_path = new PathGenerator(owner);

		float dist = i_offset ? i_offset : CONTACT_DISTANCE;
		float angle = (i_offset || i_angle != 0.f) ? i_angle : i_target->GetRelativeAngle(owner->GetPositionX(), owner->GetPositionY());

		// Aps: Custom pet posision offsets
		float dist_offset = 0;
		float angle_offset = 0;
		if (owner->ToCreature() && !owner->ToCreature()->OnBladesEdgeRopes())
		{
			player = (owner->GetCharmerOrOwner() && owner->GetCharmerOrOwner()->ToPlayer()) ? owner->GetCharmerOrOwner()->ToPlayer() : 0;
			if (player && player->GetGUID() == i_target->GetGUID() && owner->ToCreature() && owner->isPet() && owner->ToPet()->IsPermanentPetFor(player) && player->GetSpeed(MOVE_RUN) <= 12.0f & player->GetSpeedRate(MOVE_RUN) >= 0.9f)
			{
				if (player->isMoving() || owner->isInCombat())
				{
					owner->ToCreature()->StartIdleTimer();
				}
				player->GetDistAngleForPet(dist_offset, angle_offset);
			}
		}

		Position pos;
		i_target->GetFirstCollisionPosition(pos, i_target->GetObjectSize() + dist + dist_offset, angle + angle_offset);

		// Aps: Make sure pet does not go to a different level or too far from the player
		float temp_offset = dist + dist_offset;
		float offset_z = 4.0f;
		// Aps: Dalaran Sewers special condition
		if (i_target->GetMapId() == 617)
			offset_z = 3.0f;

		int i = 0;
		// Allow up to 80 attempts (+/- 4)
		while (((pos.m_positionZ + offset_z) < i_target->GetPositionZ() || (pos.m_positionZ - offset_z) > i_target->GetPositionZ()) && i <= 80)
		{
			if (i_offset >= 0)
				temp_offset -= 0.05;
			else
				temp_offset += 0.05;
			i_target->GetFirstCollisionPosition(pos, i_target->GetObjectSize() + temp_offset, angle + angle_offset);
			i++;
		}

		x = pos.m_positionX;
		y = pos.m_positionY;
		z = (i_target->IsInWater() || i_target->IsUnderWater() || i_target->IsFlying()) ? i_target->GetPositionZ() : pos.m_positionZ;
	}
    else
    {
        // the destination has not changed, we just need to refresh the path (usually speed change)
        G3D::Vector3 end = i_path->GetEndPosition();
        x = end.x;
        y = end.y;
        z = end.z;
    }

	bool result = i_path->CalculatePath(x, y, z, forceDest);
	if (!result || (i_path->GetPathType() & PATHFIND_NOPATH))
	{
		// Can't reach target
		i_recalculateTravel = true;
		if (owner->GetTypeId() == TYPEID_UNIT)
			owner->ToCreature()->SetCannotReachTarget(true);
		return;
	}

    D::_addUnitStateMove(owner);
    i_targetReached = false;
    i_recalculateTravel = false;
    owner->AddUnitState(UNIT_STATE_CHASE);
	if (owner->GetTypeId() == TYPEID_UNIT)
		owner->ToCreature()->SetCannotReachTarget(false);

    Movement::MoveSplineInit init(owner);
    init.MovebyPath(i_path->GetPath());
    init.SetWalk(static_cast<D*>(this)->EnableWalking(owner));
	// Aps: Progressive speed acceleration based on distance from destination(only for pets)
	if (player && player->GetGUID() == i_target->GetGUID())
		_updateSpeed(owner, x, y, z);
    // Using the same condition for facing target as the one that is used for SetInFront on movement end
    // - applies to ChaseMovementGenerator mostly
    init.SetFacing(i_target->GetOrientation());

    init.Launch();
}

template<class T, typename D>
bool TargetedMovementGeneratorMedium<T, D>::DoUpdate(T* owner, uint32 time_diff)
{
    if (!i_target.isValid() || !i_target->IsInWorld())
        return false;

    if (!owner || !owner->isAlive())
        return false;

    if (owner->HasUnitState(UNIT_STATE_NOT_MOVE))
    {
        D::_clearUnitStateMove(owner);
        return true;
    }

    // prevent movement while casting spells with cast time or channel time
    if (owner->HasUnitState(UNIT_STATE_CASTING))
    {
		bool stop = true;
		if (Spell* spell = owner->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
			if (!(spell->GetSpellInfo()->ChannelInterruptFlags & (AURA_INTERRUPT_FLAG_MOVE | AURA_INTERRUPT_FLAG_TURNING)) && !(spell->GetSpellInfo()->InterruptFlags & SPELL_INTERRUPT_FLAG_MOVEMENT))
				stop = false;

		if (stop)
		{
			// Mainaz: delay distance recheck in case of succeeding casts
			i_recheckDistance.Reset(300);
			if (!owner->IsStopped())
				owner->StopMoving();

			return true;
		}
    }

	// Aps: Not sure this is needed
	/*
    // prevent crash after creature killed pet
    if (static_cast<D*>(this)->_lostTarget(owner))
    {
        D::_clearUnitStateMove(owner);
        return true;
    }
	*/

    bool targetMoved = false;
	bool targetStoped = false;
    i_recheckDistance.Update(time_diff);
    if (i_recheckDistance.Passed())
    {
        i_recheckDistance.Reset(100);

        //More distance let have better performance, less distance let have more sensitive reaction at target move.
		float allowed_dist = i_offset ? i_offset : CONTACT_DISTANCE;

		if (!owner->GetMap()->IsBattlegroundOrArena())
			allowed_dist += sWorld->getRate(RATE_TARGET_POS_RECALCULATION_RANGE);
		else
			allowed_dist += CONTACT_DISTANCE;

		if (!owner->HasUnitState(UNIT_STATE_FOLLOW))
			allowed_dist += owner->GetCombatReach();

        G3D::Vector3 dest = owner->movespline->FinalDestination();

		if (owner->GetTypeId() == TYPEID_UNIT && (owner->ToCreature()->CanFly() || owner->ToCreature()->canSwim()))
			targetMoved = !i_target->IsWithinDist3d(dest.x, dest.y, dest.z, allowed_dist);
		else
			targetMoved = !i_target->IsWithinDist2d(dest.x, dest.y, allowed_dist);

		if (i_path && targetMoved)
			targetMoved = !i_path->IsOffMeshPoint(owner->movespline->currentPathIdx()) && !i_path->IsOffMeshPoint(owner->movespline->currentPathIdx() + 1);
	}

	if (i_recalculateTravel || targetMoved || (i_recalculateSpeed && (owner->GetTypeId() != TYPEID_UNIT || !owner->ToCreature()->isPet())))
	{
		_setTargetLocation(owner, targetMoved);
	}
	else if (owner->ToCreature() && !owner->ToCreature()->IsIdle() && i_target->GetSpeed(MOVE_RUN) <= 12.0f && !owner->isAttacking() && !owner->HasBreakableByDamageCrowdControlAura())
	{
		if (owner->ToCreature() && !owner->ToCreature()->OnBladesEdgeRopes())
			_setTargetLocation(owner, true);
	}

    if (owner->movespline->Finalized() && owner->ToCreature() && owner->ToCreature()->IsIdle())
    {
        static_cast<D*>(this)->MovementInform(owner);
        
		if (!i_targetReached)
		{
			i_targetReached = true;
			static_cast<D*>(this)->_reachTarget(owner);
		}

		if (!i_targetStoped)
		{
			if (owner->HasUnitState(UNIT_STATE_FOLLOW))
				_setTargetLocation(owner, true);
			i_targetStoped = true;
		}

		// If idle but player moved, re-position pet
		if (owner->ToCreature()->IsReIdle())
		{
			_setTargetLocation(owner, true);
			owner->ToCreature()->SetReIdle(false);
		}

		if (i_path)
			i_path->Clear();
    }
	else
	{
		i_targetStoped = false;
	}

    return true;
}

template<class T, typename D>
void TargetedMovementGeneratorMedium<T, D>::_updateSpeed(T* owner, float &x, float &y, float &z)
{
	Unit* player = owner->GetCharmerOrOwner();
	if (!player || player->isInCombat() || player->GetSpeed(MOVE_RUN) > 12.0f)
		return;

	// Pet only sync speed with owner
	// Make sure we are not in the process of a map change (IsInWorld)
	if (!IS_PLAYER_GUID(owner->GetOwnerGUID()) || !owner->IsInWorld() || !i_target.isValid() || i_target->GetGUID() != owner->GetOwnerGUID())
		return;

	// Get pet move rate
	float moveRunRate = owner->GetSpeedRate(MOVE_RUN);
	float moveWalkRate = owner->GetSpeedRate(MOVE_WALK);
	float moveSwimRate = owner->GetSpeedRate(MOVE_SWIM);

	if (owner->isPet() && !owner->isInCombat())
	{

		// distance from player
		float distance = owner->GetDistance(x, y, z);
		float maxAccelerationDistance = 17.0f; // = 20 yards
		float distanceToAccelerate = 3.0f;

		// Only increase speed if distance from owner is > 5 yards
		if (distance > distanceToAccelerate)
		{
			// Acceleration increases from 0.0 - 1.0 based on the distance between the pet and the owner
			float acceleration = (distance - distanceToAccelerate) / maxAccelerationDistance;

			moveRunRate += acceleration;
			moveWalkRate += acceleration;
			moveSwimRate += acceleration;
		}

		if (!player->IsWalking() && player->HasUnitMovementFlag(MOVEMENTFLAG_BACKWARD) && moveWalkRate < 1.7f)
		{
			moveWalkRate += 1.7f;
		}

		// Aps: Security Cap
		if (moveRunRate > 2.0f)
			moveRunRate = 2.0f;
		if (moveWalkRate > 2.0f)
			moveWalkRate = 2.0f;
		if (moveSwimRate > 2.0f)
			moveSwimRate = 2.0f;

		// Aps: Decelerate when close to owner in order to stay next to him
		float decelerate = 1.0;
		if (owner->IsWithinRange(player, 0.5f))
		{
			float decelerate = 0.95;
			moveRunRate *= decelerate;
			moveWalkRate *= decelerate;
			moveSwimRate *= decelerate;
		}

		if (owner->GetSpeedRate(MOVE_RUN) != moveRunRate)
			owner->SetSpeed(MOVE_RUN, moveRunRate, true);
		if (owner->GetSpeedRate(MOVE_WALK) != moveWalkRate)
			owner->SetSpeed(MOVE_WALK, moveWalkRate, true);
		if (owner->GetSpeedRate(MOVE_SWIM) != moveSwimRate)
			owner->SetSpeed(MOVE_SWIM, moveSwimRate, true);
	}
}

//-----------------------------------------------//
template<class T>
void ChaseMovementGenerator<T>::_reachTarget(T* owner)
{
    _clearUnitStateMove(owner);
    if (owner->IsWithinMeleeRange(this->i_target.getTarget()))
        owner->Attack(this->i_target.getTarget(), true);
}

template<>
void ChaseMovementGenerator<Player>::DoInitialize(Player* owner)
{
    owner->AddUnitState(UNIT_STATE_CHASE | UNIT_STATE_CHASE_MOVE);
    _setTargetLocation(owner, true);
}

template<>
void ChaseMovementGenerator<Creature>::DoInitialize(Creature* owner)
{
    owner->SetWalk(false);
    owner->AddUnitState(UNIT_STATE_CHASE | UNIT_STATE_CHASE_MOVE);
    _setTargetLocation(owner, true);
}

template<class T>
void ChaseMovementGenerator<T>::DoFinalize(T* owner)
{
    owner->ClearUnitState(UNIT_STATE_CHASE | UNIT_STATE_CHASE_MOVE);
}

template<class T>
void ChaseMovementGenerator<T>::DoReset(T* owner)
{
    DoInitialize(owner);
}

template<class T>
void ChaseMovementGenerator<T>::MovementInform(T* /*unit*/) { }

template<>
void ChaseMovementGenerator<Creature>::MovementInform(Creature* unit)
{
    // Pass back the GUIDLow of the target. If it is pet's owner then PetAI will handle
    if (unit->AI())
        unit->AI()->MovementInform(CHASE_MOTION_TYPE, i_target.getTarget()->GetGUIDLow());
}

//-----------------------------------------------//
template<>
bool FollowMovementGenerator<Creature>::EnableWalking(Creature* owner) const
{
	return i_target->IsWithinMeleeRange(owner) && i_target.isValid() && (i_target->IsWalking() || i_target->HasUnitMovementFlag(MOVEMENTFLAG_BACKWARD));
}

template<>
bool FollowMovementGenerator<Player>::EnableWalking(Player * /*owner*/) const
{
    return false;
}

template<>
void FollowMovementGenerator<Player>::DoInitialize(Player* owner)
{
    owner->AddUnitState(UNIT_STATE_FOLLOW | UNIT_STATE_FOLLOW_MOVE);
    _setTargetLocation(owner, true);
}

template<>
void FollowMovementGenerator<Creature>::DoInitialize(Creature* owner)
{
    owner->AddUnitState(UNIT_STATE_FOLLOW | UNIT_STATE_FOLLOW_MOVE);
    _setTargetLocation(owner, true);
}

template<class T>
void FollowMovementGenerator<T>::DoFinalize(T* owner)
{
    owner->ClearUnitState(UNIT_STATE_FOLLOW | UNIT_STATE_FOLLOW_MOVE);
	owner->StopMoving();
}

template<class T>
void FollowMovementGenerator<T>::DoReset(T* owner)
{
    DoInitialize(owner);
}

template<class T>
void FollowMovementGenerator<T>::MovementInform(T* /*unit*/) { }

template<>
void FollowMovementGenerator<Creature>::MovementInform(Creature* unit)
{
    // Pass back the GUIDLow of the target. If it is pet's owner then PetAI will handle
    if (unit->AI())
        unit->AI()->MovementInform(FOLLOW_MOTION_TYPE, i_target.getTarget()->GetGUIDLow());
}

//-----------------------------------------------//
template void TargetedMovementGeneratorMedium<Player, ChaseMovementGenerator<Player> >::_setTargetLocation(Player*, bool);
template void TargetedMovementGeneratorMedium<Player, FollowMovementGenerator<Player> >::_setTargetLocation(Player*, bool);
template void TargetedMovementGeneratorMedium<Creature, ChaseMovementGenerator<Creature> >::_setTargetLocation(Creature*, bool);
template void TargetedMovementGeneratorMedium<Creature, FollowMovementGenerator<Creature> >::_setTargetLocation(Creature*, bool);
template bool TargetedMovementGeneratorMedium<Player, ChaseMovementGenerator<Player> >::DoUpdate(Player*, uint32);
template bool TargetedMovementGeneratorMedium<Player, FollowMovementGenerator<Player> >::DoUpdate(Player*, uint32);
template bool TargetedMovementGeneratorMedium<Creature, ChaseMovementGenerator<Creature> >::DoUpdate(Creature*, uint32);
template bool TargetedMovementGeneratorMedium<Creature, FollowMovementGenerator<Creature> >::DoUpdate(Creature*, uint32);

template void ChaseMovementGenerator<Player>::_reachTarget(Player*);
template void ChaseMovementGenerator<Creature>::_reachTarget(Creature*);
template void ChaseMovementGenerator<Player>::DoFinalize(Player*);
template void ChaseMovementGenerator<Creature>::DoFinalize(Creature*);
template void ChaseMovementGenerator<Player>::DoReset(Player*);
template void ChaseMovementGenerator<Creature>::DoReset(Creature*);
template void ChaseMovementGenerator<Player>::MovementInform(Player*);

template void FollowMovementGenerator<Player>::DoFinalize(Player*);
template void FollowMovementGenerator<Creature>::DoFinalize(Creature*);
template void FollowMovementGenerator<Player>::DoReset(Player*);
template void FollowMovementGenerator<Creature>::DoReset(Creature*);
template void FollowMovementGenerator<Player>::MovementInform(Player*);