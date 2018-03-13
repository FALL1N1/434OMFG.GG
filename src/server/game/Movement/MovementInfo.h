#ifndef _MOVEMENT_INFO_H_
#define _MOVEMENT_INFO_H_

#include "Opcodes.h"
#include "WorldPacket.h"

class Unit;

struct Position;

struct MovementInfo
{
    // common
    uint64 guid;
    uint32 flags;
    uint16 flags2;
    Position pos;
    uint32 time;
    // transport
    uint64 t_guid;
    Position t_pos;
    int8 t_seat;
    uint32 t_time;
    uint32 t_time2;
    uint32 t_time3;
    // swimming/flying
    float pitch;
    // falling
    uint32 fallTime;
    // jumping
    float j_zspeed, j_cosAngle, j_sinAngle, j_xyspeed;
    // spline
    bool hasSpline;
    float splineElevation;
    // ack
    uint32 ackCount;
    float ackSpeed;
    // height
    float height;

    MovementInfo();

    uint32 GetMovementFlags() const { return flags; }
    void SetMovementFlags(uint32 flag) { flags = flag; }
    void AddMovementFlag(uint32 flag) { flags |= flag; }
    void RemoveMovementFlag(uint32 flag) { flags &= ~flag; }
    bool HasMovementFlag(uint32 flag) const { return flags & flag; }

    uint16 GetExtraMovementFlags() const { return flags2; }
    void AddExtraMovementFlag(uint16 flag) { flags2 |= flag; }
    bool HasExtraMovementFlag(uint16 flag) const { return flags2 & flag; }

    void SetFallTime(uint32 time) { fallTime = time; }

	void ClearTransport()
 	{
 		t_guid = 0;
 		t_pos.Relocate(0.0f, 0.0f, 0.0f, 0.0f);
 		t_seat = -1;
 		t_time = 0;
 		t_time2 = 0;
 	}

    void OutDebug();

    void Write_OLD(WorldPacket &data);
    void Write_SMSG_MOVE_UPDATE_KNOCK_BACK(ByteBuffer &buff);
    void Write_SMSG_MOVE_TELEPORT(ByteBuffer &buff);

    void ReadFromPacket(WorldPacket &packet);
    void WriteToPacket(WorldPacket &packet);
    void Sanitize(Unit *target);

    Opcodes GetSMSGOpcodeForCMSG(Opcodes cmsg);

    void Read_OLD(WorldPacket &data);
    void Read_CMSG_MOVE_KNOCK_BACK_ACK(ByteBuffer &buff);
};

#endif
