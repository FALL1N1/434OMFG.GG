#include "ScriptMgr.h"
#include "Player.h"

class DuelResetScript : public PlayerScript
 {
    public:
        DuelResetScript() : PlayerScript("DuelResetScript") { }

        void OnDuelEnd(Player* winner, Player* loser, DuelCompleteType type) override
        {
            if (type != DUEL_WON)
                return;

            if (sWorld->getBoolConfig(CONFIG_RESET_ARENA_COOLDOWNS_AFTER_DUEL))
            {
                winner->RemoveArenaSpellCooldowns(true);
                loser->RemoveArenaSpellCooldowns(true);
            }
        }
};

void AddSC_duel_reset()
{
    new DuelResetScript();
}