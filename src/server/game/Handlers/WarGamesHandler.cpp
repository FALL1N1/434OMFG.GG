#include "Common.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Object.h"

#include "Battlefield.h"
#include "BattlefieldMgr.h"
#include "Opcodes.h"
#include "Player.h"

void WorldSession::HandleWarGameCommentatorStart(WorldPacket& recvData)
{

}

void WorldSession::HandleWarGameAccept(WorldPacket& recvData)
{
    // empty opcode
}

void WorldSession::HandleWarGameStart(WorldPacket& recvData)
{
    ObjectGuid BattlegroundGUID, TargetPlayerGUID;

    TargetPlayerGUID[0] = recvData.ReadBit();
    TargetPlayerGUID[7] = recvData.ReadBit();
    BattlegroundGUID[3] = recvData.ReadBit();
    BattlegroundGUID[7] = recvData.ReadBit();
    BattlegroundGUID[1] = recvData.ReadBit();
    TargetPlayerGUID[5] = recvData.ReadBit();
    TargetPlayerGUID[1] = recvData.ReadBit();
    TargetPlayerGUID[2] = recvData.ReadBit();
    BattlegroundGUID[6] = recvData.ReadBit();
    BattlegroundGUID[5] = recvData.ReadBit();
    BattlegroundGUID[2] = recvData.ReadBit();
    BattlegroundGUID[0] = recvData.ReadBit();
    BattlegroundGUID[4] = recvData.ReadBit();
    TargetPlayerGUID[4] = recvData.ReadBit();
    TargetPlayerGUID[3] = recvData.ReadBit();
    TargetPlayerGUID[6] = recvData.ReadBit();

    recvData.ReadByteSeq(TargetPlayerGUID[6]);
    recvData.ReadByteSeq(BattlegroundGUID[7]);
    recvData.ReadByteSeq(BattlegroundGUID[3]);
    recvData.ReadByteSeq(TargetPlayerGUID[4]);
    recvData.ReadByteSeq(BattlegroundGUID[5]);
    recvData.ReadByteSeq(BattlegroundGUID[2]);
    recvData.ReadByteSeq(TargetPlayerGUID[1]);
    recvData.ReadByteSeq(TargetPlayerGUID[3]);
    recvData.ReadByteSeq(TargetPlayerGUID[5]);
    recvData.ReadByteSeq(BattlegroundGUID[0]);
    recvData.ReadByteSeq(TargetPlayerGUID[2]);
    recvData.ReadByteSeq(TargetPlayerGUID[7]);
    recvData.ReadByteSeq(BattlegroundGUID[6]);
    recvData.ReadByteSeq(TargetPlayerGUID[0]);
    recvData.ReadByteSeq(BattlegroundGUID[1]);
    recvData.ReadByteSeq(BattlegroundGUID[4]);
}

void WorldSession::HandleWarGameCheckEntry(uint64 )
{
    WorldPacket response(SMSG_WARGAME_REQUEST_SENT);
    uint8 byteMask[] = { 6, 0, 3, 5, 2, 1, 4, 7 };
    uint8 byteBytes[] = { 5, 6, 3, 0, 7, 4, 2, 1 };
    response.WriteGuidMask(/*battlegroundGuid*/0, byteMask, 8);
    response.WriteGuidBytes(/*battlegroundGuid*/0, byteBytes, 8, 0);

    //leaderChallenger->GetSession()->SendPacket(&response);
}

void WorldSession::HandleWarGameRequestSent(uint64 )
{

}
