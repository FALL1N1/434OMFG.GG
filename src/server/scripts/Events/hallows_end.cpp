/*
 * Copyright
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

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellAuras.h"
#include "Player.h"

enum Npcs
{
    NPC_STINKY_BOMB_CREDIT              = 15415,
    NPC_HEADLESS_FIRE                   = 23537,
    NPC_FIRE_DUMMY                      = 23686,
    NPC_SHADE_HORSEMAN                  = 23543,
    NPC_WICKERMAN_TORCH_HORDE           = 51684,
    NPC_WICKERMAN_TORCH_ALLIANCE        = 51720,
    NPC_CRONE                           = 53762,
    NPC_CRONES_BROOM_HORDE              = 53761,
    NPC_CRONES_BROOM_ALLIANCE           = 51710,
    NPC_GERTRUDE_FENLOW                 = 52548,
};

enum Gobs
{
    GO_STINKY_BOMB_FLASK                = 180449,
    GO_STINKY_BOMB_CLOUD                = 208200,
    GO_FIRE_EFFIGY                      = 186720,
    GO_LARGE_JACK_O_LANTERN             = 186887
};

enum quests
{
    QUEST_CRASHING_WICKERMAN_FESTIVAL   = 1658,
    QUEST_LET_THE_FIRES_COME_A          = 12135,
    QUEST_LET_THE_FIRES_COME_H          = 12139,
    QUEST_STOP_FIRES_A                  = 11131,
    QUEST_STOP_FIRES_H                  = 11219
};

enum Spells
{
    SPELL_PIRATE_COSTUME_MALE           = 24708,
    SPELL_PIRATE_COSTUME_FEMALE         = 24709,
    SPELL_NINJA_COSTUME_MALE            = 24711,
    SPELL_NINJA_COSTUME_FEMALE          = 24710,
    SPELL_LEPER_GNOME_COSTUME_MALE      = 24712,
    SPELL_LEPER_GNOME_COSTUME_FEMALE    = 24713,
    SPELL_GHOST_COSTUME_MALE            = 24735,
    SPELL_GHOST_COSTUME_FEMALE          = 24736,
    SPELL_HALLOWEEN_WAND_PIRATE         = 24717,
    SPELL_HALLOWEEN_WAND_NINJA          = 24718,
    SPELL_HALLOWEEN_WAND_LEPER_GNOME    = 24719,
    SPELL_HALLOWEEN_WAND_RANDOM         = 24720,
    SPELL_HALLOWEEN_WAND_SKELETON       = 24724,
    SPELL_HALLOWEEN_WAND_WISP           = 24733,
    SPELL_HALLOWEEN_WAND_GHOST          = 24737,
    SPELL_HALLOWEEN_WAND_BAT            = 24741,
    SPELL_GRIM_VISAGE                   = 24705,
    SPELL_FIRE_CREATE_NODE              = 42118,
    SPELL_WATER_SPOUT_VISUAL            = 42348,
    SPELL_FIRE_VISUAL_BUFF              = 42074,
    SPELL_FIRE_SIZE_STACK               = 42091,
    SPELL_FIRE_STARTING_SIZE            = 42096,
    SPELL_QUESTS_CREDITS                = 42242,
    SPELL_CREATE_WATER_BUCKET           = 42349,
    SPELL_HORSEMAN_CONFLAGRATION        = 42380,
    SPELL_HORSEMAN_JACK_O_LANTERN       = 44185,
    SPELL_HORSEMAN_CLEAVE               = 15496,
    SPELL_THROW_TORCH                   = 95958,

    // Stink Bombs away
    SPELL_RIDE_VEHICLE_HARDCODED        = 46598,
    SPELL_HALLOWS_END_SUMMON_HORDE      = 100042,
    SPELL_HALLOWS_END_SUMMON_ALLIANCE   = 96020,
    SPELL_TELEPORT_TO_STORMWIND         = 100028,
    SPELL_TELEPORT_TO_UNDERCITY         = 96005,
    SPELL_CLEAN_STINKY_BOMB_CREDIT      = 97157,

    // Goldshire / Razor hill event
    SPELL_BUCKET_LANDS                  = 42339,
};

enum Talks
{
    TALK_SUMMONED           = -22022,
    TALK_2_MIN_PAST         = -22035,
    TALK_2_MIN_30_SEC_PAST  = -22034,
    TALK_EVENT_FAILED       = -22026,
    TALK_EVENT_DONE         = -22027,
    TALK_DEATH              = -22757,
    EMOTE_CONFLAGRATION     = -22587
};

enum Events
{
    EVENT_FIRE_HIT_BY_BUCKET        = 1,
    EVENT_FIRE_GROW_FIRE            = 2,
    EVENT_HORSEMAN_CONFLAGRATION    = 3,
    EVENT_HORSEMAN_CLEAVE           = 4,
    EVENT_FIRE_FAIL                 = 5,
    EVENT_START_PATH                = 6,
    EVENT_RETURN_PATH               = 7,
    EVENT_TALK_2_MIN                = 8,
    EVENT_TALK_2_MIN_30_SEC         = 9,
    EVENT_START_TELEPORT            = 10,
};

enum Actions
{
    ACTION_RETURN_HOME  = 1,
};

enum Points
{
    POINT_TELEPORT_TO_CITY  = 1,
    POINT_DESPAWN           = 2,
    POINT_GROUND            = 1337
};

#define FIRE_NODES_PER_AREA  13

const Position FireNodesGoldShire[FIRE_NODES_PER_AREA + 1] =
{
    {-9459.41f, 43.90f, 64.23f, 0.00f},
    {-9472.57f, 41.11f, 64.17f, 0.00f},
    {-9467.22f, 85.86f, 66.20f, 0.00f},
    {-9472.94f, 93.84f, 69.20f, 0.00f},
    {-9462.50f, 103.90f, 68.51f, 0.00f},
    {-9467.84f, 100.69f, 66.12f, 0.00f},
    {-9456.91f, 112.81f, 66.12f, 0.00f},
    {-9478.22f, 41.65f, 69.85f, 0.00f},
    {-9481.30f, 24.87f, 69.08f, 0.00f},
    {-9482.69f, 14.39f, 62.94f, 0.00f},
    {-9471.16f, -6.65f, 70.76f, 0.00f},
    {-9451.26f, 38.83f, 68.02f, 0.00f},
    {-9450.13f, 89.46f, 66.22f, 0.00f},
    {-9464.28f,68.1982f,56.2331f,0.0f}, // Center of Town
};

const Position FireNodesRazorHill[FIRE_NODES_PER_AREA+1] =
{
    {372.70f, -4714.64f, 23.11f, 0.00f},
    {343.11f, -4708.87f, 29.19f, 0.00f},
    {332.06f, -4703.21f, 24.52f, 0.00f},
    {317.20f, -4694.22f, 16.78f, 0.00f},
    {326.30f, -4693.24f, 34.59f, 0.00f},
    {281.18f, -4705.37f, 22.38f, 0.00f},
    {293.32f, -4773.45f, 25.03f, 0.00f},
    {280.17f, -4831.90f, 22.25f, 0.00f},
    {319.04f, -4770.23f, 31.47f, 0.00f},
    {362.50f, -4676.11f, 28.63f, 0.00f},
    {348.71f, -4805.08f, 32.23f, 0.00f},
    {342.88f, -4837.07f, 26.29f, 0.00f},
    {361.80f, -4769.27f, 18.49f, 0.00f},
    {317.837f,-4734.06f,9.76272f,0.0f}, // Center of Town
};

#define GOSSIP_WICKERMAN_EMBER "Smear the ash on my face like war paint!"

uint32 const StinkBombsAwayQuestHordePathSize = 79;
const G3D::Vector3 StinkBombsAwayQuestHordePath[StinkBombsAwayQuestHordePathSize] =
{
    { -8967.150f, 512.1090f, 146.9610f },
    { -8926.179f, 513.5434f, 146.0898f },
    { -8928.653f, 527.4549f, 149.1052f },
    { -8922.420f, 543.9774f, 152.4396f },
    { -8913.674f, 553.1389f, 150.9259f },
    { -8898.094f, 565.8611f, 146.4797f },
    { -8886.982f, 578.4011f, 131.8325f },
    { -8869.654f, 590.8715f, 117.3132f },
    { -8853.676f, 606.4028f, 117.3132f },
    { -8844.554f, 624.1320f, 117.3132f },
    { -8852.786f, 642.1545f, 117.3132f },
    { -8852.450f, 654.8125f, 117.3132f },
    { -8843.466f, 669.1389f, 117.3132f },
    { -8829.913f, 676.3680f, 125.1623f },
    { -8811.113f, 680.8281f, 125.1623f },
    { -8794.721f, 692.4496f, 132.4432f },
    { -8785.123f, 721.7257f, 134.7661f },
    { -8799.667f, 746.6719f, 127.0066f },
    { -8814.748f, 785.5243f, 113.6912f },
    { -8827.380f, 817.5608f, 114.7905f },
    { -8826.250f, 842.1996f, 114.7905f },
    { -8818.226f, 872.2969f, 114.7905f },
    { -8788.938f, 881.9011f, 114.7905f },
    { -8745.493f, 884.1441f, 114.7905f },
    { -8703.314f, 891.4670f, 114.7905f },
    { -8672.504f, 908.4236f, 114.7905f },
    { -8631.123f, 918.2483f, 114.7905f },
    { -8609.933f, 905.7743f, 114.7905f },
    { -8592.455f, 883.0208f, 114.7905f },
    { -8608.976f, 846.1024f, 114.7905f },
    { -8634.094f, 817.0295f, 114.7905f },
    { -8629.212f, 787.8750f, 114.7905f },
    { -8629.723f, 757.0573f, 118.6781f },
    { -8631.821f, 728.4636f, 123.9302f },
    { -8617.967f, 711.4688f, 120.7624f },
    { -8601.147f, 717.4479f, 118.9534f },
    { -8578.191f, 738.0573f, 114.7905f },
    { -8553.924f, 765.4583f, 125.6543f },
    { -8532.451f, 771.4340f, 123.3722f },
    { -8511.969f, 763.8906f, 114.7905f },
    { -8485.469f, 747.7795f, 114.7905f },
    { -8469.127f, 735.0364f, 119.7802f },
    { -8441.823f, 720.4948f, 123.9715f },
    { -8427.113f, 708.4080f, 122.1203f },
    { -8414.910f, 692.5000f, 122.1203f },
    { -8403.578f, 664.4479f, 122.1203f },
    { -8392.290f, 641.1111f, 124.3749f },
    { -8371.946f, 624.8750f, 124.7377f },
    { -8348.292f, 617.6233f, 122.1203f },
    { -8335.897f, 635.5660f, 122.1203f },
    { -8352.518f, 655.1840f, 122.1203f },
    { -8377.870f, 651.5903f, 120.5722f },
    { -8403.196f, 633.1146f, 117.3857f },
    { -8434.196f, 608.2361f, 116.8653f },
    { -8458.250f, 586.2604f, 115.7205f },
    { -8474.147f, 568.7361f, 116.2499f },
    { -8492.386f, 543.6788f, 118.6645f },
    { -8487.448f, 509.0729f, 122.1203f },
    { -8503.814f, 494.2379f, 120.5533f },
    { -8526.323f, 487.7240f, 122.1203f },
    { -8541.524f, 476.6371f, 122.1203f },
    { -8542.823f, 460.0712f, 122.1203f },
    { -8541.752f, 441.9774f, 122.1203f },
    { -8558.516f, 432.0885f, 122.1203f },
    { -8578.643f, 435.7708f, 122.1203f },
    { -8603.877f, 444.3090f, 122.1203f },
    { -8625.169f, 447.5156f, 122.1203f },
    { -8643.450f, 449.8299f, 125.1388f },
    { -8677.507f, 448.5087f, 127.1211f },
    { -8696.103f, 431.2604f, 127.6389f },
    { -8714.389f, 422.8246f, 130.9356f },
    { -8735.772f, 427.4705f, 139.4648f },
    { -8752.978f, 444.0121f, 139.4648f },
    { -8764.692f, 457.1806f, 139.4648f },
    { -8789.797f, 466.1059f, 139.4648f },
    { -8816.232f, 472.1962f, 145.9180f },
    { -8843.864f, 473.4722f, 147.2900f },
    { -8885.976f, 476.8038f, 150.9953f },
    { -8915.364f, 498.5295f, 146.3356f }
};

uint32 const StinkBombsAwayQuestAlliancePathSize = 75;
const G3D::Vector3 StinkBombsAwayQuestAlliancePath[StinkBombsAwayQuestAlliancePathSize] =
{
    { 1717.285f, 348.2413f, 20.23495f },
    { 1724.509f, 337.2031f, 11.80331f },
    { 1735.102f, 314.3559f, -20.56344f },
    { 1747.181f, 283.5851f, -40.61856f },
    { 1752.826f, 255.1372f, -43.82628f },
    { 1738.186f, 242.1267f, -47.13151f },
    { 1704.476f, 239.9063f, -52.75056f },
    { 1669.595f, 241.3403f, -52.75056f },
    { 1663.993f, 235.1736f, -56.44384f },
    { 1660.054f, 218.0972f, -56.44384f },
    { 1647.606f, 195.8403f, -56.44384f },
    { 1618.122f, 175.2569f, -56.44384f },
    { 1591.681f, 172.4983f, -56.44384f },
    { 1562.734f, 176.6528f, -56.44384f },
    { 1533.047f, 206.7066f, -56.44384f },
    { 1524.212f, 233.3333f, -56.44384f },
    { 1529.760f, 261.5816f, -56.44384f },
    { 1546.182f, 286.5035f, -56.44384f },
    { 1576.585f, 307.6285f, -56.44384f },
    { 1615.399f, 306.1354f, -56.44384f },
    { 1641.585f, 291.1267f, -56.44384f },
    { 1662.649f, 263.1996f, -56.44384f },
    { 1664.774f, 231.9115f, -56.44384f },
    { 1657.422f, 206.0260f, -56.44384f },
    { 1638.375f, 201.2500f, -56.44384f },
    { 1627.198f, 208.2517f, -56.44384f },
    { 1621.915f, 213.9583f, -56.75116f },
    { 1616.583f, 217.9462f, -55.60745f },
    { 1599.887f, 218.6094f, -50.92685f },
    { 1579.885f, 220.2483f, -46.27073f },
    { 1571.024f, 236.6684f, -52.70364f },
    { 1579.030f, 264.2621f, -43.70478f },
    { 1611.262f, 256.9670f, -41.95214f },
    { 1617.380f, 240.3542f, -41.95214f },
    { 1613.135f, 223.0816f, -41.95214f },
    { 1593.880f, 213.6042f, -41.95214f },
    { 1578.345f, 221.6771f, -41.95214f },
    { 1569.002f, 239.7674f, -41.95214f },
    { 1577.858f, 259.4375f, -39.65142f },
    { 1595.896f, 261.2222f, -39.65142f },
    { 1613.816f, 257.1996f, -39.65142f },
    { 1627.106f, 240.4635f, -32.83950f },
    { 1637.733f, 239.8663f, -30.19070f },
    { 1657.977f, 239.6632f, -33.54613f },
    { 1671.554f, 239.4201f, -34.84373f },
    { 1682.774f, 239.5365f, -34.84373f },
    { 1695.816f, 240.3976f, -50.25166f },
    { 1717.266f, 240.2188f, -50.25166f },
    { 1731.050f, 239.4115f, -50.25166f },
    { 1741.766f, 232.6267f, -50.25166f },
    { 1752.262f, 221.6840f, -50.25166f },
    { 1749.050f, 204.6823f, -53.26390f },
    { 1738.727f, 176.0174f, -53.26390f },
    { 1718.658f, 144.6754f, -53.26390f },
    { 1692.665f, 117.0955f, -53.26390f },
    { 1658.623f, 95.73959f, -53.26390f },
    { 1600.681f, 79.30382f, -53.26390f },
    { 1561.740f, 67.78646f, -55.49710f },
    { 1541.318f, 66.75174f, -55.49710f },
    { 1525.113f, 76.61459f, -54.08760f },
    { 1505.840f, 93.54688f, -50.04650f },
    { 1495.708f, 126.6840f, -53.32158f },
    { 1486.210f, 170.4306f, -57.18730f },
    { 1464.542f, 205.4566f, -58.12598f },
    { 1441.189f, 255.1719f, -58.12598f },
    { 1459.012f, 316.9445f, -58.12598f },
    { 1481.467f, 348.1684f, -58.12598f },
    { 1518.076f, 375.6910f, -58.12598f },
    { 1558.950f, 393.0938f, -56.09516f },
    { 1595.616f, 397.3837f, -56.09516f },
    { 1625.023f, 394.8906f, -43.34521f },
    { 1647.694f, 389.7448f, -32.69637f },
    { 1666.976f, 381.7483f, -18.29517f },
    { 1679.797f, 375.4844f, -6.709838f },
    { 1693.816f, 362.8368f, 19.92288f }
};

class spell_halloween_wand : public SpellScriptLoader
{
public:
    spell_halloween_wand() : SpellScriptLoader("spell_halloween_wand") {}

    class spell_halloween_wand_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_halloween_wand_SpellScript)

        bool Validate(SpellInfo const* /*spellEntry*/) override
        {
            if (!sSpellMgr->GetSpellInfo(SPELL_PIRATE_COSTUME_MALE))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_PIRATE_COSTUME_FEMALE))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_NINJA_COSTUME_MALE))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_NINJA_COSTUME_FEMALE))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_LEPER_GNOME_COSTUME_MALE))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_LEPER_GNOME_COSTUME_FEMALE))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_GHOST_COSTUME_MALE))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_GHOST_COSTUME_FEMALE))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_HALLOWEEN_WAND_PIRATE))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_HALLOWEEN_WAND_NINJA))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_HALLOWEEN_WAND_LEPER_GNOME))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_HALLOWEEN_WAND_RANDOM))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_HALLOWEEN_WAND_SKELETON))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_HALLOWEEN_WAND_WISP))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_HALLOWEEN_WAND_GHOST))
                return false;
            if (!sSpellMgr->GetSpellInfo(SPELL_HALLOWEEN_WAND_BAT))
                return false;
            return true;
        }

        void HandleScriptEffect()
        {
            Unit* caster = GetCaster();
            Unit* target = GetHitPlayer();

            if (!caster || !target)
                return;

            uint32 spellId = 0;
            uint8 gender = target->getGender();

            switch (GetSpellInfo()->Id)
            {
                case SPELL_HALLOWEEN_WAND_LEPER_GNOME:
                    spellId = gender ? SPELL_LEPER_GNOME_COSTUME_FEMALE : SPELL_LEPER_GNOME_COSTUME_MALE;
                    break;
                case SPELL_HALLOWEEN_WAND_PIRATE:
                    spellId = gender ? SPELL_PIRATE_COSTUME_FEMALE : SPELL_PIRATE_COSTUME_MALE;
                    break;
                case SPELL_HALLOWEEN_WAND_GHOST:
                    spellId = gender ? SPELL_GHOST_COSTUME_FEMALE : SPELL_GHOST_COSTUME_MALE;
                    break;
                case SPELL_HALLOWEEN_WAND_NINJA:
                    spellId = gender ? SPELL_NINJA_COSTUME_FEMALE : SPELL_NINJA_COSTUME_MALE;
                    break;
                case SPELL_HALLOWEEN_WAND_RANDOM:
                    spellId = RAND(SPELL_HALLOWEEN_WAND_PIRATE, SPELL_HALLOWEEN_WAND_NINJA, SPELL_HALLOWEEN_WAND_LEPER_GNOME, SPELL_HALLOWEEN_WAND_SKELETON, SPELL_HALLOWEEN_WAND_WISP, SPELL_HALLOWEEN_WAND_GHOST, SPELL_HALLOWEEN_WAND_BAT);
                    break;
            }
            caster->CastSpell(target, spellId, true);
        }

        void Register() override
        {
            AfterHit += SpellHitFn(spell_halloween_wand_SpellScript::HandleScriptEffect);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_halloween_wand_SpellScript();
    }
};

class spell_toss_stinky_bomb : public SpellScriptLoader
{
public:
    spell_toss_stinky_bomb() : SpellScriptLoader("spell_toss_stinky_bomb") {}

    class spell_toss_stinky_bomb_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_toss_stinky_bomb_SpellScript)

        void HandleScriptEffect(SpellEffIndex effIndex)
        {
            Unit* caster = GetCaster();

            if (caster && caster->GetTypeId() == TYPEID_PLAYER)
                caster->ToPlayer()->KilledMonsterCredit(NPC_STINKY_BOMB_CREDIT, 0);
        }

        void Register() override
        {
            OnEffectHit += SpellEffectFn(spell_toss_stinky_bomb_SpellScript::HandleScriptEffect, EFFECT_2, SPELL_EFFECT_SEND_EVENT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_toss_stinky_bomb_SpellScript();
    }
};

class spell_clean_stinky_bomb_trigger : public SpellScriptLoader
{
public:
    spell_clean_stinky_bomb_trigger() : SpellScriptLoader("spell_clean_stinky_bomb_trigger") {}

    class spell_clean_stinky_bomb_trigger_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_clean_stinky_bomb_trigger_SpellScript)

        SpellCastResult CheckIfNearBomb()
        {
            WorldLocation const* const dest = GetExplTargetDest();
            std::list<GameObject*> stinkyBombList;
            GetCaster()->GetGameObjectListWithEntryInGrid(stinkyBombList, GO_STINKY_BOMB_CLOUD, 15.0f);
            bool nearStinkyBomb = false;
            for (GameObject* stinkyCloud : stinkyBombList)
            {
                if (stinkyCloud->GetDistance2d(dest->GetPositionX(), dest->GetPositionY()) <= 4.00f)
                {
                    if (stinkyCloud->GetGoState() == GO_STATE_READY)
                    {
                        nearStinkyBomb = true;
                        break;
                    }
                }
            }

            if (nearStinkyBomb)
                return SPELL_CAST_OK;
            else
                return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
        }

        void Register() override
        {
            OnCheckCast += SpellCheckCastFn(spell_clean_stinky_bomb_trigger_SpellScript::CheckIfNearBomb);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_clean_stinky_bomb_trigger_SpellScript();
    }
};

class spell_clean_stinky_bomb : public SpellScriptLoader
{
public:
    spell_clean_stinky_bomb() : SpellScriptLoader("spell_clean_stinky_bomb") {}

    class spell_clean_stinky_bomb_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_clean_stinky_bomb_SpellScript)

        void HandleCleanBombEffect(SpellEffIndex effIndex)
        {
            WorldLocation const* const dest = GetExplTargetDest();
            std::list<GameObject*> stinkyBombList;
            GetCaster()->GetGameObjectListWithEntryInGrid(stinkyBombList, GO_STINKY_BOMB_CLOUD, 15.0f);
            for (GameObject* stinkyCloud : stinkyBombList)
            {
                if (stinkyCloud->GetDistance2d(dest->GetPositionX(), dest->GetPositionY()) <= 4.00f)
                {
                    stinkyCloud->SetGoState(GO_STATE_ACTIVE);
                    stinkyCloud->RemoveFromWorld();
                    GetCaster()->CastSpell(GetCaster(), SPELL_CLEAN_STINKY_BOMB_CREDIT, true);
                    break;
                }
            }
        }

        void Register() override
        {
            OnEffectHit += SpellEffectFn(spell_clean_stinky_bomb_SpellScript::HandleCleanBombEffect, EFFECT_0, SPELL_EFFECT_ACTIVATE_OBJECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_clean_stinky_bomb_SpellScript();
    }
};

class at_wickerman_festival : public AreaTriggerScript
{
public:
    at_wickerman_festival() : AreaTriggerScript("at_wickerman_festival") {}

    bool OnTrigger(Player* player, AreaTriggerEntry const* /*trigger*/) override
    {
        player->GroupEventHappens(QUEST_CRASHING_WICKERMAN_FESTIVAL, player);
        return true;
    }
};

class go_wickerman_ember : public GameObjectScript
{
public:
    go_wickerman_ember() : GameObjectScript("go_wickerman_ember") { }

    bool OnGossipHello(Player* player, GameObject* go) override
    {
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_WICKERMAN_EMBER, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
        player->SEND_GOSSIP_MENU(player->GetGossipTextId(go), go->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, GameObject* go, uint32 /*sender*/, uint32 action) override
    {
        if (action == GOSSIP_ACTION_INFO_DEF)
        {
            player->CastSpell(player, SPELL_GRIM_VISAGE, true);
            if (player->GetQuestStatus(29376) == QUEST_STATUS_INCOMPLETE)
                player->KilledMonsterCredit(53827);
        }

        player->PlayerTalkClass->ClearMenus();
        player->CLOSE_GOSSIP_MENU();
        return true;
    }
};

class npc_halloween_fire : public CreatureScript
{
public:
    npc_halloween_fire() : CreatureScript("npc_halloween_fire") { }

    struct npc_halloween_fireAI : public ScriptedAI
    {
        npc_halloween_fireAI(Creature* creature) : ScriptedAI(creature) {}

        void InitializeAI() override
        {
            if (GameObject* effigy = me->FindNearestGameObject(GO_FIRE_EFFIGY, 0.5f))
                effigy->SetGoState(GO_STATE_ACTIVE);
            else
            {
                DoCast(me, SPELL_FIRE_STARTING_SIZE, true);
                DoCast(me, SPELL_FIRE_VISUAL_BUFF, true);
                me->CastCustomSpell(SPELL_FIRE_SIZE_STACK, SPELLVALUE_AURA_STACK, 41, me, TRIGGERED_FULL_MASK);
            }
            events.ScheduleEvent(EVENT_FIRE_GROW_FIRE, 1000);
        }

        void SpellHit(Unit* attacker, SpellInfo const* spellInfo) override
        {
            if (spellInfo->Id == SPELL_BUCKET_LANDS)
            {
                if (GameObject* effigy = me->FindNearestGameObject(GO_FIRE_EFFIGY, 0.5f))
                {
                    if (effigy->GetGoState() == GO_STATE_ACTIVE)
                    {
                        if (attacker->GetTypeId() == TYPEID_PLAYER)
                            attacker->ToPlayer()->KilledMonsterCredit(me->GetEntry());
                        effigy->SetGoState(GO_STATE_READY);
                        events.ScheduleEvent(EVENT_FIRE_GROW_FIRE, 7500);
                    }
                }
                else
                {
                    if (Creature* horseman = me->GetCreature(*me, me->GetCreatorGUID()))
                        horseman->AI()->SetGUID(attacker->GetGUID(), EVENT_FIRE_HIT_BY_BUCKET);

                    if (Aura* fireSize = me->GetAura(SPELL_FIRE_SIZE_STACK))
                    {
                        int32 stacks = fireSize->GetStackAmount();
                        if ((stacks - 20) <= 20)
                            me->DespawnOrUnsummon(2000);
                        else
                            fireSize->ModStackAmount(((stacks > 100) ? -40 : -20));
                    }
                }
            }
        }

        void UpdateAI(const uint32 diff) override
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_FIRE_GROW_FIRE:
                        if (GameObject* effigy = me->FindNearestGameObject(GO_FIRE_EFFIGY, 0.5f))
                            effigy->SetGoState(GO_STATE_ACTIVE);
                        else
                        {
                            if (Aura* fireSize = me->GetAura(SPELL_FIRE_SIZE_STACK))
                                if (fireSize->GetStackAmount() < 255)
                                    me->CastSpell(me, SPELL_FIRE_SIZE_STACK, true);
                            events.ScheduleEvent(EVENT_FIRE_GROW_FIRE, urand(1500, 2500));
                        }
                        break;
                    default:
                        break;
                }
            }
        }

    private:
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_halloween_fireAI(creature);
    }
};

class npc_shade_horseman : public CreatureScript
{
public:
    npc_shade_horseman() : CreatureScript("npc_shade_horseman") { }


    struct npc_shade_horsemanAI : public ScriptedAI
    {
        npc_shade_horsemanAI(Creature* creature) : ScriptedAI(creature), summons(creature) {}

        void InitializeAI() override
        {
            moving = true;
            pointReached = true;
            allFiresSet = false;
            firesOut = false;
            wpCount = 0;
            _playerList.clear();

            me->Mount(25159);
            me->SetReactState(REACT_PASSIVE);
            me->SetCanFly(true);
            me->SetHover(true);
            ScriptedAI::InitializeAI();
        }

        void Reset()
        {
            events.Reset();
            events.ScheduleEvent(EVENT_HORSEMAN_CONFLAGRATION, urand(7000, 14000));
            events.ScheduleEvent(EVENT_TALK_2_MIN, 120000);
            events.ScheduleEvent(EVENT_TALK_2_MIN_30_SEC, 150000);
            events.ScheduleEvent(EVENT_FIRE_FAIL, 600000);
        }

        void JustDied(Unit* killer) override
        {
            Talk(TALK_DEATH);
            killer->SummonGameObject(GO_LARGE_JACK_O_LANTERN, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 1.00f, me->GetOrientation(), 0.0f, 0.0f, 0.0f, 0.0f, 180000);
        }

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
        }

        void SummonedCreatureDespawn(Creature* summon)
        {
            summons.Despawn(summon);
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type != POINT_MOTION_TYPE)
                return;

            if (moving && id < FIRE_NODES_PER_AREA)
            {
                if (id == 0 && !allFiresSet)
                    Talk(TALK_SUMMONED);

                if (!allFiresSet)
                {
                    const Position pos = GetPositionsForArea()[wpCount];
                    if (pos.IsPositionValid())
                        me->CastSpell(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), SPELL_FIRE_CREATE_NODE, true);
                }

                if (id+1 == FIRE_NODES_PER_AREA)
                {
                    allFiresSet = true;
                    wpCount = 0;
                }
                else
                    ++wpCount;

                pointReached = true;
            }
            else if (id == POINT_GROUND)
            {
                me->SetReactState(REACT_AGGRESSIVE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NON_ATTACKABLE);
                me->SetCanFly(false);
                me->SetHover(false);
                me->Dismount();
                events.ScheduleEvent(EVENT_HORSEMAN_CLEAVE, urand(5000, 10000));
            }
        }

        void UpdateAI(const uint32 diff) override
        {
            if (firesOut && !UpdateVictim())
                return;

            events.Update(diff);

            if (!firesOut)
            {
                const Position pos = GetPositionsForArea()[wpCount];

                if (!pos.IsPositionValid())
                    return;

                if (pointReached)
                {
                    pointReached = false;
                    me->GetMotionMaster()->MovePoint(wpCount, pos.GetPositionX(), pos.GetPositionY(), GetZForArea());
                }

                if (allFiresSet && summons.empty())
                {
                    firesOut = true;
                    Talk(TALK_EVENT_DONE);
                    DoCast(me, SPELL_QUESTS_CREDITS, true);
                    events.CancelEvent(EVENT_FIRE_FAIL);
                    events.CancelEvent(EVENT_TALK_2_MIN);
                    events.CancelEvent(EVENT_TALK_2_MIN_30_SEC);
                    me->GetMotionMaster()->Clear();
                    if (me->GetMapId() == 0)
                    {
                        Position pos(-9459.720f, 63.198f, 55.833f, me->GetOrientation());
                        me->SetHomePosition(pos);
                        me->GetMotionMaster()->MovePoint(POINT_GROUND, pos);
                    }
                    else
                    {
                        Position pos(313.820f, -4741.687f, 9.558f, me->GetOrientation());
                        me->SetHomePosition(pos);
                        me->GetMotionMaster()->MovePoint(POINT_GROUND, pos);
                    }
                    return;
                }
            }

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_FIRE_FAIL:
                        summons.DespawnAll();
                        Talk(TALK_EVENT_FAILED);
                        for (std::list<uint64>::const_iterator i = _playerList.begin(); i != _playerList.end(); ++i)
                        {
                            if (Player* player = me->GetPlayer(*me, *i))
                            {
                                uint32 questId = player->GetTeam() == ALLIANCE ? QUEST_LET_THE_FIRES_COME_A : QUEST_LET_THE_FIRES_COME_H;
                                if (player->GetQuestStatus(questId) == QUEST_STATUS_INCOMPLETE)
                                    player->FailQuest(questId);
                            }
                        }
                        me->DespawnOrUnsummon(5000);
                        break;
                    case EVENT_HORSEMAN_CONFLAGRATION:
                        for (std::list<uint64>::const_iterator i = _playerList.begin(); i != _playerList.end(); ++i)
                        {
                            Player* player = me->GetPlayer(*me, *i);
                            if (player && player->GetDistance(me) <= 30.0f)
                            {
                                me->SendPlaySound(11975, false);
                                Talk(EMOTE_CONFLAGRATION, 0, CHAT_MSG_TEXT_EMOTE);
                                DoCast(player, SPELL_HORSEMAN_CONFLAGRATION, true);
                                break;
                            }
                        }
                        events.ScheduleEvent(EVENT_HORSEMAN_CONFLAGRATION, urand(10000, 15000));
                        break;
                    case EVENT_HORSEMAN_CLEAVE:
                        DoCastVictim(SPELL_HORSEMAN_CLEAVE, true);
                        events.ScheduleEvent(EVENT_HORSEMAN_CLEAVE, urand(5000, 10000));
                        return;
                    case EVENT_TALK_2_MIN:
                        Talk(TALK_2_MIN_PAST);
                        break;
                    case EVENT_TALK_2_MIN_30_SEC:
                        Talk(TALK_2_MIN_30_SEC_PAST);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }

        const Position* GetPositionsForArea()
        {
            switch (me->GetAreaId())
            {
                case 87: //GoldShire
                    return FireNodesGoldShire;
                case 362: // Razor Hill
                case 2337:
                case 14:
                    return FireNodesRazorHill;
            }
            return NULL;
        }

        float GetZForArea()
        {
            switch (me->GetAreaId())
            {
                case 87: //GoldShire
                    return 77.6f;
                case 362: // Razor Hill
                case 2337:
                case 14:
                    return 40.0f;
            }
            return 0.0f;
        }

        void SetGUID(uint64 guid, int32 id) override
        {
            if (id == EVENT_FIRE_HIT_BY_BUCKET)
            {
                _playerList.push_back(guid);
                _playerList.unique();
            }

        }

        private:
            SummonList summons;
            EventMap events;
            bool moving;
            bool pointReached;
            bool allFiresSet;
            bool firesOut;
            uint32 wpCount;
            std::list<uint64> _playerList;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_shade_horsemanAI(creature);
    }
};

class npc_halloween_orphan_matron : public CreatureScript
{
public:
    npc_halloween_orphan_matron() : CreatureScript("npc_halloween_orphan_matron")
    {
        _headlessHoresemanGUID = 0;
    }

    uint64 _headlessHoresemanGUID;

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (creature->isQuestGiver())
        {
            QuestRelationBounds objectQR = sObjectMgr->GetCreatureQuestRelationBounds(creature->GetEntry());
            QuestRelationBounds objectQIR = sObjectMgr->GetCreatureQuestInvolvedRelationBounds(creature->GetEntry());

            QuestMenu& qm = player->PlayerTalkClass->GetQuestMenu();
            qm.ClearMenu();

            for (QuestRelations::const_iterator i = objectQIR.first; i != objectQIR.second; ++i)
            {
                uint32 questId = i->second;
                QuestStatus status = player->GetQuestStatus(questId);
                if (status == QUEST_STATUS_COMPLETE)
                    qm.AddMenuItem(questId, 4);
                else if (status == QUEST_STATUS_INCOMPLETE)
                    qm.AddMenuItem(questId, 4);
            }

            for (QuestRelations::const_iterator i = objectQR.first; i != objectQR.second; ++i)
            {
                uint32 questId = i->second;
                Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
                if (!quest)
                    continue;

                switch (questId)
                {
                    case QUEST_LET_THE_FIRES_COME_A:
                    case QUEST_LET_THE_FIRES_COME_H:
                    {
                        QuestStatus status = player->GetQuestStatus(questId);
                        if (status == QUEST_STATUS_NONE && player->CanTakeQuest(quest, false))
                            if ((player->GetOTeam() == ALLIANCE && player->GetQuestStatus(QUEST_STOP_FIRES_A) == QUEST_STATUS_NONE) || (player->GetOTeam() == HORDE && player->GetQuestStatus(QUEST_STOP_FIRES_H) == QUEST_STATUS_NONE))
                            if (!Unit::GetCreature(*creature, _headlessHoresemanGUID))
                                qm.AddMenuItem(questId, 2);
                        break;
                    }
                    case QUEST_STOP_FIRES_A:
                    case QUEST_STOP_FIRES_H:
                    {
                        QuestStatus status = player->GetQuestStatus(questId);
                        if (status == QUEST_STATUS_NONE && player->CanTakeQuest(quest, false))
                            if ((player->GetOTeam() == ALLIANCE && player->GetQuestStatus(QUEST_LET_THE_FIRES_COME_A) == QUEST_STATUS_NONE) || (player->GetOTeam() == HORDE && player->GetQuestStatus(QUEST_LET_THE_FIRES_COME_H) == QUEST_STATUS_NONE))
                            if (Unit::GetCreature(*creature, _headlessHoresemanGUID))
                                qm.AddMenuItem(questId, 2);
                        break;
                    }
                    default:
                        QuestStatus status = player->GetQuestStatus(questId);
                        if (status == QUEST_STATUS_NONE && player->CanTakeQuest(quest, false))
                            qm.AddMenuItem(questId, 2);
                        break;
                    }
                }
            }
            player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
            return true;
    }

    bool OnQuestAccept(Player* player, Creature* me, Quest const* quest) override
    {
        if (!(me->GetAreaId() == 87 || me->GetAreaId() == 362))
            return true;

        if (quest->GetQuestId() == QUEST_LET_THE_FIRES_COME_A || quest->GetQuestId() == QUEST_LET_THE_FIRES_COME_H)
        {
            Creature* horseman = ObjectAccessor::GetCreature(*me, _headlessHoresemanGUID);

            if (!horseman)
                horseman = me->FindNearestCreature(NPC_SHADE_HORSEMAN, 500.00f);

            if (!horseman || !horseman->isAlive())
            {
                if (horseman && !horseman->isAlive())
                    horseman->DespawnOrUnsummon();

                if (Creature* newHorseman = player->SummonCreature(NPC_SHADE_HORSEMAN, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 20.0f, 0, TEMPSUMMON_DEAD_DESPAWN, 180000))
                    _headlessHoresemanGUID = newHorseman->GetGUID();
            }
        }
        return true;
    }
};

class npc_halloween_crone_broom : public CreatureScript
{
public:
    npc_halloween_crone_broom() : CreatureScript("npc_halloween_crone_broom") { }


    struct npc_halloween_crone_broomAI : public ScriptedAI
    {
        npc_halloween_crone_broomAI(Creature* creature) : ScriptedAI(creature) {}

        void InitializeAI() override
        {
            isHordeBroom = me->GetEntry() == NPC_CRONES_BROOM_HORDE;
            ScriptedAI::InitializeAI();
        }

        void IsSummonedBy(Unit* summoner) override
        {
            summoner->EnterVehicle(me, 1);
            me->setActive(true);
            me->SetSpeed(MOVE_FLIGHT, 2.00f);
        }

        void PassengerBoarded(Unit* passenger, int8 seat, bool apply) override
        {
            if (passenger->GetTypeId() != TYPEID_PLAYER)
                return;

            if (apply)
            {
                if (isHordeBroom)
                    me->GetMotionMaster()->MovePoint(POINT_TELEPORT_TO_CITY, 1695.579f, 592.5800f, 83.1840f);
                else
                    me->GetMotionMaster()->MovePoint(POINT_TELEPORT_TO_CITY, -9051.566f, 455.046f, 112.8316f);
            }              
        }

        void OnCharmed(bool /*apply*/) override {}

        void DoAction(int32 const action)
        {
            if (action == ACTION_RETURN_HOME)
            {
                if (isHordeBroom)
                {
                    me->RemoveAurasDueToSpell(SPELL_TELEPORT_TO_STORMWIND);
                    me->GetMap()->LoadGrid(1695.579f, 592.5800f);
                    me->NearTeleportTo(1695.579f, 592.5800f, 83.1840f, 5.010f, true);
                }
                else
                {
                    me->RemoveAurasDueToSpell(SPELL_TELEPORT_TO_UNDERCITY);
                    me->GetMap()->LoadGrid(-9038.202f, 445.330f);
                    me->NearTeleportTo(-9038.202f, 445.330f, 112.728f, 3.597f, true);
                }
                events.ScheduleEvent(EVENT_RETURN_PATH, 1000);
            }
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type == POINT_MOTION_TYPE)
            {
                switch (id)
                {
                    case POINT_TELEPORT_TO_CITY:
                        if (isHordeBroom)
                            me->GetMap()->LoadGrid(-8967.15f, 512.109f);
                        else
                            me->GetMap()->LoadGrid(1707.640f, 357.827f);
                        events.ScheduleEvent(EVENT_START_TELEPORT, 200);
                        break;
                    case POINT_DESPAWN:
                        me->DespawnOrUnsummon();
                        break;
                    default:
                        break;
                }
            }

            if (type == SPLINE_MOTION_TYPE)
            {
                if (isHordeBroom)
                {
                    if (id == (StinkBombsAwayQuestHordePathSize - 1))
                        me->GetMotionMaster()->MoveSmoothPath(StinkBombsAwayQuestHordePath, StinkBombsAwayQuestHordePathSize);
                }
                else
                {
                    if (id == (StinkBombsAwayQuestAlliancePathSize - 1))
                        me->GetMotionMaster()->MoveSmoothPath(StinkBombsAwayQuestAlliancePath, StinkBombsAwayQuestAlliancePathSize);
                }
            }
        }

        void UpdateAI(uint32 const diff) override
        {
            events.Update(diff);
            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_START_PATH:
                        for (int8 seat = 0; seat < 8; seat++)
                            if (Unit* passenger = me->GetVehicleKit()->GetPassenger(seat))
                                if (Player* player = passenger->ToPlayer())
                                {
                                    player->SetMover(me);
                                    if (isHordeBroom)
                                    {
                                        Position pos(-8967.150f, 512.1090f, 146.9610f, 0.6455f);
                                        player->UpdatePosition(pos, true);
                                    }
                                    else
                                    {
                                        Position pos(1707.640f, 357.827f, 17.7164f, 6.2732f);
                                        player->UpdatePosition(pos, true);
                                    }
                                }

                        if (isHordeBroom)
                        {
                            // Reapply Teleport to Stormwind aura on Vehicle seems the client dont get the aura update because he was teleported later - fixes spell casting on vehicle
                            me->RemoveAurasDueToSpell(SPELL_TELEPORT_TO_STORMWIND);
                            me->AddAura(SPELL_TELEPORT_TO_STORMWIND, me);
                            me->GetMotionMaster()->MoveSmoothPath(StinkBombsAwayQuestHordePath, StinkBombsAwayQuestHordePathSize);
                        }
                        else
                        {
                            // Reapply Teleport to Undercity aura on Vehicle seems the client dont get the aura update because he was teleported later - fixes spell casting on vehicle
                            me->RemoveAurasDueToSpell(SPELL_TELEPORT_TO_UNDERCITY);
                            me->AddAura(SPELL_TELEPORT_TO_UNDERCITY, me);
                            me->GetMotionMaster()->MoveSmoothPath(StinkBombsAwayQuestAlliancePath, StinkBombsAwayQuestAlliancePathSize);
                        }
                        break;
                    case EVENT_RETURN_PATH:
                        for (int8 seat = 0; seat < 8; seat++)
                            if (Unit* passenger = me->GetVehicleKit()->GetPassenger(seat))
                                if (Player* player = passenger->ToPlayer())
                                {
                                    if (isHordeBroom)
                                    {
                                        Position pos(1695.579f, 592.5800f, 83.1840f, 5.010f);
                                        player->UpdatePosition(pos, true);
                                    }
                                    else
                                    {
                                        Position pos(-9038.202f, 445.330f, 112.728f, 3.597f);
                                        player->UpdatePosition(pos, true);
                                    }
                                }

                        if (isHordeBroom)
                            me->GetMotionMaster()->MovePoint(POINT_DESPAWN, 1720.719f, 510.649f, 39.357f);
                        else
                            me->GetMotionMaster()->MovePoint(POINT_DESPAWN, -9113.018f, 415.5278f, 96.103f);
                        break;
                    case EVENT_START_TELEPORT:
                        if (isHordeBroom)
                            DoCast(me, SPELL_TELEPORT_TO_STORMWIND);
                        else
                            DoCast(me, SPELL_TELEPORT_TO_UNDERCITY);
                        events.ScheduleEvent(EVENT_START_PATH, 1000);
                        break;
                    default:
                        break;
                }
            }
        }

    private:
        EventMap events;
        bool isHordeBroom;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_halloween_crone_broomAI(creature);
    }
};

class npc_halloween_crina_fenlow : public CreatureScript
{
public:
    npc_halloween_crina_fenlow() : CreatureScript("npc_halloween_crina_fenlow")
    {
    }

    bool OnGossipHello(Player* player, Creature* me) override
    {
        player->PrepareQuestMenu(me->GetGUID());
        if (player->GetQuestStatus(29374) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "I'm ready to attack Stormwind!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        if (player->GetQuestStatus(29054) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "I'm ready to attack Undercity!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(me), me->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature * /*pCreature*/, uint32 /*sender*/, uint32 action) override
    {
        if (action == GOSSIP_ACTION_INFO_DEF + 1)
        {
            // We use summoncreature because the summontype? from this spell breaks vehicle enter on summer...?!
            // player->CastSpell(player, SPELL_HALLOWS_END_SUMMON_HORDE, true);

            Position pos(*player);
            player->SummonCreature(NPC_CRONES_BROOM_HORDE, pos);
        }

        if (action == GOSSIP_ACTION_INFO_DEF + 2)
        {
            Position pos(*player);
            player->SummonCreature(NPC_CRONES_BROOM_ALLIANCE, pos);
        }

        player->PlayerTalkClass->ClearMenus();
        player->CLOSE_GOSSIP_MENU();
        return true;
    }
};

class go_hallowsend_bonefire : public GameObjectScript
{
public:
    go_hallowsend_bonefire() : GameObjectScript("go_hallowsend_bonefire") { }

    bool OnGossipHello(Player* player, GameObject* go)
    {
        if (!player->HasUnitState(UNIT_STATE_CASTING))
            player->CastSpell(player, SPELL_THROW_TORCH, false);
        return true;
    }
};

class spell_hallowsend_throw_torch : public SpellScriptLoader
{
public:
    spell_hallowsend_throw_torch() : SpellScriptLoader("spell_hallowsend_throw_torch") {}

    class spell_hallowsend_throw_torch_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hallowsend_throw_torch_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if([](WorldObject* target){
                return target->GetEntry() != NPC_WICKERMAN_TORCH_HORDE && target->GetEntry() != NPC_WICKERMAN_TORCH_ALLIANCE;
            });
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hallowsend_throw_torch_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_hallowsend_throw_torch_SpellScript();
    }
};

class spell_hallowsend_return_home : public SpellScriptLoader
{
public:
    spell_hallowsend_return_home() : SpellScriptLoader("spell_hallowsend_return_home") { }

    class spell_hallowsend_return_home_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hallowsend_return_home_SpellScript);

        void HandleScriptEffect(SpellEffIndex /*effIndex*/)
        {
            if (GetCaster()->GetTypeId() == TYPEID_PLAYER)
            {
                if (GetCaster()->GetVehicleBase())
                    GetCaster()->GetVehicleBase()->ToCreature()->AI()->DoAction(ACTION_RETURN_HOME);
            }
            else if (GetCaster()->GetTypeId() == TYPEID_UNIT)
                GetCaster()->ToCreature()->AI()->DoAction(ACTION_RETURN_HOME);
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_hallowsend_return_home_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_hallowsend_return_home_SpellScript();
    }
};

class spell_hallowsend_bucket_lands : public SpellScriptLoader
{
public:
    spell_hallowsend_bucket_lands() : SpellScriptLoader("spell_hallowsend_bucket_lands") { }

    class spell_hallowsend_bucket_lands_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_hallowsend_bucket_lands_SpellScript);

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            WorldLocation const* dest = GetExplTargetDest();
            GetCaster()->CastSpell(dest->GetPositionX(), dest->GetPositionY(), dest->GetPositionZ(), SPELL_WATER_SPOUT_VISUAL, true);
        }

        void Register()
        {
            OnEffectHit += SpellEffectFn(spell_hallowsend_bucket_lands_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_hallowsend_bucket_lands_SpellScript();
    }
};

void AddSC_event_hallows_end()
{
    new spell_toss_stinky_bomb();
    new spell_clean_stinky_bomb_trigger();
    new spell_clean_stinky_bomb();
    new at_wickerman_festival();
    new spell_halloween_wand();
    new go_wickerman_ember();
    new npc_halloween_fire();
    new npc_shade_horseman();
    new npc_halloween_orphan_matron();
    new npc_halloween_crone_broom();
    new npc_halloween_crina_fenlow();
    new go_hallowsend_bonefire();
    new spell_hallowsend_throw_torch();
    new spell_hallowsend_return_home();
    new spell_hallowsend_bucket_lands();
}
