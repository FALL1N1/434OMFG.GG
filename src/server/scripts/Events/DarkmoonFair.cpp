
#include "AchievementMgr.h"
#include "GameEventMgr.h"
#include "Group.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "SpellScript.h"

enum spells
{
    // troll cogne attraction
    SPELL_COGNE_TROLL                = 110230, // visual bar
    SPELL_MALLET_GOSSIP              = 102189, // trigger 101612 // requiert gob 301115
    SPELL_COGNE_TROLL_2              = 101612, // cognez ! 101604
    SPELL_WACK_SUMMON_AURA           = 101994, // concel ride aura, casted by polayer on player trigger 105011
    SPELL_WHACK_SUM_GNOLL_HOLDER     = 101995, // cast by player, hit 54546
    SPELL_SWACK_SUMMON_VISUAL        = 102136, // casted by 54444
    SPELL_SWACK_SUMMON               = 102036, // summon 54444 hit 54547 casted by player
    SPELL_SWACK_BABY                 = 102043, // summon 54466 hit 54547 casted by player
    SPELL_OK_TO_HIT                  = 101996, // casted by player hit 54547
    SPELL_WACK_EXPLOSION_DUMMY       = 101640,
    SPELL_COGNER_DUMMY_CONE          = 102022,
    SPELL_COGNER                     = 101604,
    SPELL_WACK_GNOLL_KILL_CREDIT     = 101835,
    SPELL_FORBIDDEN_ACCESS_TROLL     = 110966,
    SPELL_BAD_TARGET                 = 101679,

    // canonball
    SPELL_CANON_STUN = 109332,
    SPELL_CANON_PREPARATION_TELEPORT = 102113, // dest db
    SPELL_CANON_PREPARATION = 102112, // bomb visual
    SPELL_MAGIC_WINGS = 102116,
    SPELL_CANON_SHOT = 102115, // hit 15218
    SPELL_CANON_DEFLAG = 102121, // visual bar
    SPELL_CANCEL_WINGS = 102120,
    SPELL_DND_CANCEL_SHAPESHIFT_MOUNT = 105011,
    SPELL_CANON_RETURN = 109244,
    SPELL_LANDING_RESULT_KILLCREDIT = 100962,

    // chariot attraction
    //chariot
    SPELL_FORBIDDEN_ACCESS_CHARIOT     = 109976,
    SPELL_AURA_RESEARCH = 102222, // casted by 54642 (chariot)
    //mini zep
    SPELL_BOMB_AURA = 102209, // trigger 102210
    SPELL_BOMB_LAUNCH = 102210, // casted by 54643 (mini zep)
    SPELL_BOMB = 102211,

    SPELL_TINK_TARGET_TRACKET_CREDITS = 110162, // kill credit, dummy, energize

    SPELL_TONK_GOSSIP = 102192,
    SPELL_TONK_OBJECTIVES_VISUAL = 102178,
    SPELL_TONK_SUMMON_RIDE = 100752,
    SPELL_TONK_SUM = 102190, // requier gob 301116

    SPELL_TONK_CREDITS = 62265,


    // quick shot attraction
    SPELL_QUICK_SHOT_OVEERIDE_SPELLS = 101871, // visul bar rquiert gob 300129
    SPELL_INDICATOR_QUICK_SHOT = 101010,
    SPELL_INDICATOR_QUICK_SHOT_VISUAL = 43313, // from sniff
    SPELL_QUICK_SHOT_KILLCREDITS = 101012,
    SPELL_QUICK_SHOT = 101872,

    // ring shoot attraction
    SPELL_RING_LAUNCH_OVERRIDE_SPELLS = 102058, // visual bar, requier gob 301114
    SPELL_FORBIDDEN_ACCESS_RING        = 109972,
    SPELL_RING_LAUNCH_1 = 109765,
    SPELL_RING_LAUNCH_2 = 101696,
    SPELL_RING_VISUAL = 101738,
    SPELL_RING_LAUNCH_KILLCREDITS = 101807,
    // misc
    SPELL_DND_SCARED                 = 98842, // casted by 68426
    SPELL_SOUFFLEUR_FEU              = 102910,

};

enum quests
{
    // troll cogne attraction
    QUEST_GNOLL                     = 29463,
    QUEST_CANNON_BALL               = 29436,
    QUEST_TARGET_TURTLE             = 29455,
    QUES_QUICK_SHOOT                = 29438,
    QUEST_TONK_COMMANDER            = 29434,
};

enum Achievements
{
    ACHIEVEMENT_REFLEXE_SHOT = 6022,
    ACHIEVEMENT_CANON_SHOT = 6021,
    ACHIEVEMENT_DARKMOON_ASSIST = 6019,
    ACHIEVEMENT_RIGHT_UP = 6020, // play 5 differents game
};

#define    DARKMOON_MONEY             71083

#define    GOSSIP_GNOLL_START           "Ready to whack! |cFF0008E8(Darkmoon Game Token)|r"
#define    GOSSIP_GNOLL_INFORMATION     "How do I play Whack-a-Gnoll?"

class npc_darkmoon_mallet : public CreatureScript
{
public:
    npc_darkmoon_mallet() : CreatureScript("npc_darkmoon_mallet") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
        case GOSSIP_ACTION_INFO_DEF:
            if (player->HasItemCount(DARKMOON_MONEY))
            {
                player->CastSpell(player, SPELL_MALLET_GOSSIP, true);
                player->DestroyItemCount(DARKMOON_MONEY, 1, true);
            }
            player->SEND_GOSSIP_MENU(13018, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_GNOLL_INFORMATION, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->SEND_GOSSIP_MENU(13018, creature->GetGUID());
            break;
        }
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(QUEST_GNOLL) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_GNOLL_START, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_GNOLL_INFORMATION, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->SEND_GOSSIP_MENU(13018, creature->GetGUID());
        return true;
    }
};

class npc_wack_a_gnoll_barrel : public CreatureScript
{
public:
    npc_wack_a_gnoll_barrel() : CreatureScript("npc_wack_a_gnoll_barrel") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_wack_a_gnoll_barrelAI(creature);
    }

    struct npc_wack_a_gnoll_barrelAI : public ScriptedAI
    {
        npc_wack_a_gnoll_barrelAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        void Reset()
        {
            sumTimer = urand(1000, 5000);
        }

        void DoAction(int32 const action)
        {
            if (action == 42)
                sumTimer = urand(1000, 5000);
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (!who || who->GetTypeId() != TYPEID_PLAYER)
                return;
            if (me->GetDistance(who) <= 7)
                if (!who->HasAura(SPELL_COGNE_TROLL))
                    me->CastSpell(who, SPELL_FORBIDDEN_ACCESS_TROLL, true);
        }

        void UpdateAI(uint32 const diff)
        {
            if (sumTimer <= diff)
            {
                if (rand() % 6 != 0)
                    me->CastSpell(me, SPELL_SWACK_SUMMON, true);
                else
                    me->CastSpell(me, SPELL_SWACK_BABY, true);
                me->CastSpell(me, SPELL_SWACK_SUMMON_VISUAL, true);
                sumTimer = urand(12000, 14000);
            }
            else
                sumTimer -= diff;
        }

    private:
        uint32 sumTimer;
    };
};

class spell_cognez_dummy : public SpellScriptLoader
{
public:
    spell_cognez_dummy() : SpellScriptLoader("spell_cognez_dummy") { }

    class spell_cognez_dummy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_cognez_dummy_SpellScript);

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            return true;
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (Unit* caster = GetCaster())
                if (Player *player = caster->ToPlayer())
                    if (player->GetQuestStatus(QUEST_GNOLL) == QUEST_STATUS_INCOMPLETE)
                        player->CastSpell(player, SPELL_COGNER_DUMMY_CONE, true);
        }

        void Register()
        {
            OnEffectHit += SpellEffectFn(spell_cognez_dummy_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_cognez_dummy_SpellScript();
    }
};


class spell_cognez_dummy_cone : public SpellScriptLoader
{
public:
    spell_cognez_dummy_cone() : SpellScriptLoader("spell_cognez_dummy_cone") { }

    class spell_cognez_dummy_cone_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_cognez_dummy_cone_SpellScript);

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            return true;
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (Unit* target = GetHitUnit())
                if (Creature *c = target->ToCreature())
                {
                    switch (c->GetEntry())
                    {
                        case 54444:
                            if (Unit *caster = GetCaster())
                                caster->CastSpell(caster, SPELL_WACK_GNOLL_KILL_CREDIT, true);
                            if (c->ToTempSummon())
                                if (Unit *p = c->ToTempSummon()->GetSummoner())
                                    if (Creature *summoner = c->ToTempSummon()->GetSummoner()->ToCreature())
                                        summoner->AI()->DoAction(42);
                            c->CastSpell(c, SPELL_WACK_EXPLOSION_DUMMY, true);
                            c->Kill(c);
                            break;
                        case 54466:
                            if (Unit *caster = GetCaster())
                                target->CastSpell(caster, SPELL_BAD_TARGET, true);
                            c->CastSpell(c, SPELL_WACK_EXPLOSION_DUMMY, true);
                            c->DespawnOrUnsummon(1000);
                            if (c->ToTempSummon())
                                if (Unit *p = c->ToTempSummon()->GetSummoner())
                                    if (Creature *summoner = c->ToTempSummon()->GetSummoner()->ToCreature())
                                        summoner->AI()->DoAction(42);
                            break;
                    }
                }
        }

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            std::list<Unit*> temp;
            for (std::list<WorldObject*>::iterator itr = targets.begin(); itr != targets.end(); itr++)
                if (Unit* unit = (*itr)->ToUnit())
                    temp.push_back(unit);

            targets.clear();
            for (std::list<Unit*>::iterator itr = temp.begin(); itr != temp.end(); itr++)
            {
                if ((*itr)->ToCreature())
                    if ((*itr)->ToCreature()->GetEntry() == 54444 || (*itr)->ToCreature()->GetEntry() == 54466)
                        targets.push_back((WorldObject*)(*itr));
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_cognez_dummy_cone_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_CONE_ENTRY);
            OnEffectHitTarget += SpellEffectFn(spell_cognez_dummy_cone_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_cognez_dummy_cone_SpellScript();
    }
};

#define    GOSSIP_CANON_START "Lancez-moi ! |cFF0008E8(Jeton de Sombrelune)|r"
#define    GOSSIP_CANON_INFORMATION     "Comment utiliser le canon ?"

class npc_darkmoon_cannon : public CreatureScript
{
public:
    npc_darkmoon_cannon() : CreatureScript("npc_darkmoon_cannon") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
        case GOSSIP_ACTION_INFO_DEF:
            if (player->HasItemCount(DARKMOON_MONEY))
            {
                player->DestroyItemCount(DARKMOON_MONEY, 1, true);
                player->CastSpell(player, SPELL_CANON_PREPARATION_TELEPORT, false);
                creature->CastSpell(player, SPELL_CANON_PREPARATION, false);
                creature->CastWithDelay(300, player, SPELL_MAGIC_WINGS, false);
                player->CastWithDelay(300, player, SPELL_CANON_SHOT, false);
                player->CastWithDelay(300, player, SPELL_CANON_DEFLAG, false);
                player->CastWithDelay(300, player, SPELL_CANON_STUN, false);
            }
            player->SEND_GOSSIP_MENU(13018, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_GNOLL_INFORMATION, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->SEND_GOSSIP_MENU(13018, creature->GetGUID());
            break;
        }
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(QUEST_CANNON_BALL) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_CANON_START, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_CANON_INFORMATION, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->SEND_GOSSIP_MENU(13018, creature->GetGUID());
        return true;
    }
};

class spell_magic_wings : public SpellScriptLoader
{
public:
    spell_magic_wings() : SpellScriptLoader("spell_magic_wings") { }

    class spell_magic_wings_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_magic_wings_AuraScript);

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            flyDuration = 0;
            return true;
        }

        void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            flyDuration = time(NULL);
        }

        void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            flyDuration = time(NULL) - flyDuration;
            if (Unit *target = GetTarget())
                if (flyDuration == 8)
                {
                    target->CastSpell(target, SPELL_LANDING_RESULT_KILLCREDIT, true);
                    if (Player *player = target->ToPlayer())
                        if (rand() % 3 == 0)
                            if (AchievementEntry const* camel = sAchievementMgr->GetAchievement(ACHIEVEMENT_CANON_SHOT))
                                player->CompletedAchievement(camel);
                    target->CastSpell(target, SPELL_LANDING_RESULT_KILLCREDIT, true);
                }
        }

    private :
        int64 flyDuration;

        void Register()
        {
            OnEffectApply += AuraEffectApplyFn(spell_magic_wings_AuraScript::HandleApply, EFFECT_1, SPELL_AURA_FEATHER_FALL, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
            OnEffectRemove += AuraEffectRemoveFn(spell_magic_wings_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_FEATHER_FALL, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        }

    };

    AuraScript* GetAuraScript() const
    {
        return new spell_magic_wings_AuraScript();
    }
};

#define GOSSIP_RINGLING_START_ATTR "Let's shoot !  |cFF0008E8(Jeton de Sombrelune)|r"
#define GOSSIP_RINGLING_INFORMATION "How does the Shooting Gallery work?"


class npc_darkmoon_rinling : public CreatureScript
{
public:
    npc_darkmoon_rinling() : CreatureScript("npc_darkmoon_rinling") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
        case GOSSIP_ACTION_INFO_DEF:
            if (player->HasItemCount(DARKMOON_MONEY))
            {
                player->CastSpell(player, SPELL_QUICK_SHOT_OVEERIDE_SPELLS, true);
                player->DestroyItemCount(DARKMOON_MONEY, 1, true);
            }
            player->SEND_GOSSIP_MENU(13018, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_RINGLING_INFORMATION, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->SEND_GOSSIP_MENU(13018, creature->GetGUID());
            break;
        }
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(QUES_QUICK_SHOOT) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_RINGLING_START_ATTR, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_RINGLING_INFORMATION, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->SEND_GOSSIP_MENU(13018, creature->GetGUID());
        return true;
    }

    struct npc_darkmoon_rinlingAI : public ScriptedAI
    {
        npc_darkmoon_rinlingAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        void Reset()
        {
            changeTarget = 0;
            oldTar = 0;
            targetTime = true;
        }

        void UpdateAI(uint32 const diff)
        {
            if (changeTarget <= diff)
            {
                std::list<Creature*> targetList;
                me->GetCreatureListWithEntryInGrid(targetList, 24171, 15.0f);
                int target = rand() % 3;
                int cnt = 0;
                if (targetTime)
                {
                    while (target == oldTar && cnt < 10)
                    {
                        target = rand() % 3;
                        cnt++;
                    }
                    oldTar = target;
                    cnt = 0;
                }
                for (std::list<Creature*>::const_iterator iter = targetList.begin(); iter != targetList.end(); ++iter)
                {
                    if (Creature *tar = *iter)
                    {
                        if (cnt == target && targetTime)
                        {
                            tar->CastSpell(tar, SPELL_INDICATOR_QUICK_SHOT_VISUAL, true);
                            tar->CastSpell(tar, SPELL_INDICATOR_QUICK_SHOT, true);
                        }
                        else
                            tar->RemoveAura(SPELL_INDICATOR_QUICK_SHOT_VISUAL);
                    }
                    cnt++;
                }
                changeTarget = 3000;
                targetTime = !targetTime;
            }
            else
                changeTarget -= diff;
        }

    private:
        uint32 changeTarget;
        uint32 oldTar;
        bool targetTime;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_darkmoon_rinlingAI(creature);
    }
};

class spell_quick_shot_dummy : public SpellScriptLoader
{
public:
    spell_quick_shot_dummy() : SpellScriptLoader("spell_quick_shot_dummy") { }

    class spell_quick_shot_dummy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_quick_shot_dummy_SpellScript);

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            return true;
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (Unit* caster = GetCaster())
                if (Player *player = caster->ToPlayer())
                    if (Unit* target = GetHitUnit())
                    {
                        if (player->GetQuestStatus(QUES_QUICK_SHOOT) == QUEST_STATUS_INCOMPLETE)
                        {
                            if (target->HasAura(SPELL_INDICATOR_QUICK_SHOT))
                            {
                                if (AchievementEntry const* camel = sAchievementMgr->GetAchievement(ACHIEVEMENT_REFLEXE_SHOT))
                                    player->CompletedAchievement(camel);
                                    player->CastSpell(player, SPELL_QUICK_SHOT_KILLCREDITS, true);
                                    player->CastSpell(player, SPELL_QUICK_SHOT_KILLCREDITS, true);
                            }
                            else if (target->HasAura(SPELL_INDICATOR_QUICK_SHOT_VISUAL))
                                player->CastSpell(player, SPELL_QUICK_SHOT_KILLCREDITS, true);
                        }
                    }
        }

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            std::list<Unit*> temp;
            for (std::list<WorldObject*>::iterator itr = targets.begin(); itr != targets.end(); itr++)
                if (Unit* unit = (*itr)->ToUnit())
                    temp.push_back(unit);

            targets.clear();
            for (std::list<Unit*>::iterator itr = temp.begin(); itr != temp.end(); itr++)
            {
                if ((*itr)->ToCreature())
                    if ((*itr)->ToCreature()->GetEntry() == 24171)
                        if (Unit* caster = GetCaster())
                        {
                            Position pos;
                            (*itr)->ToCreature()->GetPosition(&pos);
                            if (caster->HasInArc(static_cast<float>(M_PI / 12), &pos))
                                targets.push_back((WorldObject*)(*itr));
                        }
            }
        }

        void Register()
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_quick_shot_dummy_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_CONE_ENTRY);
            OnEffectHitTarget += SpellEffectFn(spell_quick_shot_dummy_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_quick_shot_dummy_SpellScript();
    }
};

#define GOSSIP_ROGER_START "Ready to play !  |cFF0008E8(Jeton de Sombrelune)|r"
#define GOSSIP_ROGER_INFORMATION "How do I play the Ring Toss?"

class npc_darkmoon_rogers : public CreatureScript
{
public:
    npc_darkmoon_rogers() : CreatureScript("npc_darkmoon_rogers") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
        case GOSSIP_ACTION_INFO_DEF:
            if (player->HasItemCount(DARKMOON_MONEY))
            {
                player->DestroyItemCount(DARKMOON_MONEY, 1, true);
                player->CastSpell(player, SPELL_RING_LAUNCH_OVERRIDE_SPELLS, false);
            }
            player->SEND_GOSSIP_MENU(13018, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ROGER_INFORMATION, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->SEND_GOSSIP_MENU(13018, creature->GetGUID());
            break;
        }
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(QUEST_TARGET_TURTLE) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ROGER_START, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ROGER_INFORMATION, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->SEND_GOSSIP_MENU(13018, creature->GetGUID());
        return true;
    }
};

class spell_ring_launch : public SpellScriptLoader
{
public:
    spell_ring_launch() : SpellScriptLoader("spell_ring_launch") { }

    class spell_ring_launch_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_ring_launch_AuraScript);

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            return true;
        }

        void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (Unit *caster = GetCaster())
                caster->SetPower(POWER_ALTERNATE_POWER, 10);
        }

        void Register()
        {
            AfterEffectApply += AuraEffectApplyFn(spell_ring_launch_AuraScript::HandleApply, EFFECT_1, SPELL_AURA_ENABLE_ALT_POWER, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        }

    };

    AuraScript* GetAuraScript() const
    {
        return new spell_ring_launch_AuraScript();
    }
};

class spell_turtle_ring_dummy : public SpellScriptLoader
{
public:
    spell_turtle_ring_dummy() : SpellScriptLoader("spell_turtle_ring_dummy") { }

    class spell_turtle_ring_dummy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_turtle_ring_dummy_SpellScript);

        bool Validate(SpellInfo const* /*spellEntry*/)
        {
            return true;
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            Position destPos;
            GetHitDest()->GetPosition(&destPos);
            if (Unit* caster = GetCaster())
            {
                if (Player *player = caster->ToPlayer())
                {
                    if (Creature *turtle = player->FindNearestCreature(54490, 100, true))
                    {
                        if (turtle->GetDistance(destPos) <= 1.0f)
                        {
                            player->CastSpell(player, SPELL_RING_LAUNCH_KILLCREDITS, true);
                            turtle->CastSpell(turtle, SPELL_RING_VISUAL, true);
                        }
                    }
                }
            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_turtle_ring_dummy_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_turtle_ring_dummy_SpellScript();
    }
};

class npc_darkmoon_turtle : public CreatureScript
{
public:
    npc_darkmoon_turtle() : CreatureScript("npc_darkmoon_turtle") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_darkmoon_turtleAI(creature);
    }

    struct npc_darkmoon_turtleAI : public ScriptedAI
    {
        npc_darkmoon_turtleAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        void Reset()
        {
            me->GetMotionMaster()->MoveRandom(12);
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (!who || who->GetTypeId() != TYPEID_PLAYER)
                return;
            Position pos = { -4297.12f, 6293.34f, 13.12f, 0.0f };
            if (who->GetDistance(pos) <= 14.5f)
                me->CastSpell(who, SPELL_FORBIDDEN_ACCESS_RING, true);
        }

        void UpdateAI(uint32 const diff)
        {
        }
    };
};

#define GOSSIP_TONK_START "Ready to play !  |cFF0008E8(Jeton de Sombrelune)|r"
#define GOSSIP_TONK_INFORMATION "How do I play the Tonk challenge?"

class npc_darkmoon_tonk : public CreatureScript
{
public:
    npc_darkmoon_tonk() : CreatureScript("npc_darkmoon_tonk") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
        case GOSSIP_ACTION_INFO_DEF:
            if (player->HasItemCount(DARKMOON_MONEY))
            {
                player->DestroyItemCount(DARKMOON_MONEY, 1, true);
                player->CastSpell(player, 102192, false);
            }
            player->SEND_GOSSIP_MENU(13018, creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_TONK_INFORMATION, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->SEND_GOSSIP_MENU(13018, creature->GetGUID());
            break;
        }
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(QUEST_TONK_COMMANDER) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_TONK_START, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_TONK_INFORMATION, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->SEND_GOSSIP_MENU(13018, creature->GetGUID());
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_darkmoon_tonkAI(creature);
    }

    struct npc_darkmoon_tonkAI : public ScriptedAI
    {
        npc_darkmoon_tonkAI(Creature* creature) : ScriptedAI(creature)
        {
        }

        void Reset()
        {
            Position center_pos = { -4136.12f, 6312.74f, 13.12f, 0.0f };
            Position pos;
            int count = 0;
            for (int i = 0; i < 5; i++)
            {
                me->GetRandomPoint(center_pos, 12.0f, pos);
                me->SummonCreature(33081, pos, TEMPSUMMON_CORPSE_DESPAWN);
                if (count <= 2)
                    me->SummonCreature(54642, pos, TEMPSUMMON_CORPSE_DESPAWN);
                count++;
            }

            std::list<Creature*> targetList;
            me->GetCreatureListWithEntryInGrid(targetList, 54643, 200.0f);
            for (std::list<Creature*>::const_iterator itr = targetList.begin(); itr != targetList.end(); ++itr)
                if (Creature *enemyZep = *itr)
                    if (!enemyZep->HasAura(SPELL_BOMB_AURA))
                        enemyZep->CastSpell(enemyZep, SPELL_BOMB_AURA, true);
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (!who || who->GetTypeId() != TYPEID_PLAYER)
                return;

            Position center_pos = { -4136.12f, 6312.74f, 13.12f, 0.0f };
            if (who->GetDistance(center_pos) <= 12.0f)
                if (!who->HasAura(SPELL_TONK_OBJECTIVES_VISUAL) || who->GetVehicle() == NULL)
                    me->CastSpell(who, SPELL_FORBIDDEN_ACCESS_CHARIOT, true);
        }

        void JustSummoned(Creature* summon)
        {
            if (summon->GetEntry() == 54642)
            {
                summon->CastSpell(summon, SPELL_AURA_RESEARCH, true);
                summon->GetMotionMaster()->MoveRandom(12);
            }
        }

        void SummonedCreatureDies(Creature* summoned, Unit* who)
        {
            Position center_pos = { -4136.12f, 6312.74f, 13.12f, 0.0f };
            Position pos;
            me->GetRandomPoint(center_pos, 12.0f, pos);
            me->SummonCreature(summoned->GetEntry(), pos, TEMPSUMMON_CORPSE_DESPAWN);
            me->CastSpell(who, SPELL_TINK_TARGET_TRACKET_CREDITS, true);
        }

        void UpdateAI(uint32 const diff)
        {

        }

    private :
        Position center_pos;
    };
};

class spell_tonk_sum_ride : public SpellScriptLoader
{
public:
    spell_tonk_sum_ride() : SpellScriptLoader("spell_tonk_sum_ride") { }

    class spell_tonk_sum_ride_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_tonk_sum_ride_SpellScript);

        void HandleScript(SpellEffIndex effIndex)
        {
            if (Unit *target = GetHitUnit())
                if (Player *player = target->ToPlayer())
                {
                    player->CastSpell(player, SPELL_TONK_OBJECTIVES_VISUAL, false);
                    player->TeleportTo(974, -4130.18f, 6321.23f, 13.12f, 4.4f);
                    player->CastWithDelay(200, player, SPELL_TONK_SUMMON_RIDE, false);
                }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_tonk_sum_ride_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_tonk_sum_ride_SpellScript();
    }
};

static const uint16 DarkmoonFairEvents[] =
{
    111, // January
    112, // February
    113, // March
    114, // April
    115, // May
    116, // June
    117, // July
    118, // August
    119, // Semptember
    120, // October
    121, // November
    122  // December
};

static uint32 const DARKMOON_ZONEID = 5861;

class darkmoon_fair_playerscript : public PlayerScript
{
public:
    darkmoon_fair_playerscript() : PlayerScript("darkmoon_fair_playerscript") { }

    void OnUpdateZone(Player* player, uint32 newZone, uint32 /*newArea*/) override
    {
        if (newZone == DARKMOON_ZONEID && !player->isGameMaster())
        {
            for (uint16 eventId : DarkmoonFairEvents)
                if (IsEventActive(eventId))
                    return;
            player->TeleportToHomeBind();
        }
    }
};


void AddSC_event_darkmoon_fair()
{
    new npc_darkmoon_mallet();
    new npc_wack_a_gnoll_barrel();
    new spell_cognez_dummy();
    new spell_cognez_dummy_cone();
    new npc_darkmoon_cannon();
    new spell_magic_wings();
    new npc_darkmoon_rinling();
    new spell_quick_shot_dummy();
    new npc_darkmoon_rogers();
    new spell_ring_launch();
    new spell_turtle_ring_dummy();
    new npc_darkmoon_turtle();
    new npc_darkmoon_tonk();
    new spell_tonk_sum_ride();
    new darkmoon_fair_playerscript();
}
