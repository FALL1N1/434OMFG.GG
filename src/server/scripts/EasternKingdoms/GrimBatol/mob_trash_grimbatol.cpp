
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellScript.h"

/************************
**Gardien de la Nuée Azur
*************************/

#define spell_impregnation_des_arcanes 76378 // Impregnation_des_Arcanes
#define spell_malediction_de_la_nuee_azur 90659 // Malediction_de_la_Nuee_Azur

class mob_gardien_de_la_nuee_azur: public CreatureScript
{
public:
    mob_gardien_de_la_nuee_azur() : CreatureScript("mob_gardien_de_la_nuee_azur") {}

    struct mob_gardien_de_la_nuee_azurAI : public ScriptedAI
    {
        mob_gardien_de_la_nuee_azurAI(Creature *c) : ScriptedAI(c) {}

        uint32 impregnation_des_arcanes;
        uint32 malediction_de_la_nuee_azur;

        void Reset()
        {
            impregnation_des_arcanes = 15000;
            malediction_de_la_nuee_azur = 25000;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (impregnation_des_arcanes<= diff)
            {
                DoCastVictim(spell_impregnation_des_arcanes);
                impregnation_des_arcanes = 15000;
            } else impregnation_des_arcanes-= diff;

            if (malediction_de_la_nuee_azur<= diff)
            {
                DoCastVictim(spell_malediction_de_la_nuee_azur);
                malediction_de_la_nuee_azur = 25000;
            } else malediction_de_la_nuee_azur-= diff;

            DoMeleeAttackIfReady();
        }

    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new mob_gardien_de_la_nuee_azurAI(pCreature);
    }

};

/*****************************
 **Gardien de la Nuée Cramoisie
 ******************************/

#define spell_charge_cramoisie 76404 // Charge_Cramoisie
#define spell_onde_de_choc_cramoisie 76409 // Onde_de_Choc_Cramoisie

class mob_gardien_de_la_nuee_cramoisie: public CreatureScript
{
public:
    mob_gardien_de_la_nuee_cramoisie() : CreatureScript("mob_gardien_de_la_nuee_cramoisie") { }

    struct mob_gardien_de_la_nuee_cramoisieAI : public ScriptedAI
    {
        mob_gardien_de_la_nuee_cramoisieAI(Creature *c) : ScriptedAI(c) {}

        uint32 charge_cramoisie;
        uint32 onde_de_choc_cramoisie;

        void Reset()
        {
            charge_cramoisie = 16000;
            onde_de_choc_cramoisie = 25000;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (charge_cramoisie<= diff)
            {
                DoCast(SelectTarget(SELECT_TARGET_RANDOM,1,100,true), spell_charge_cramoisie);
                charge_cramoisie = 16000;
            } else charge_cramoisie-= diff;

            if (onde_de_choc_cramoisie<= diff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, -10.0f, true))
                    DoCast(target, spell_onde_de_choc_cramoisie);
                onde_de_choc_cramoisie = 25000;
            } else onde_de_choc_cramoisie-= diff;

            DoMeleeAttackIfReady();
        }

    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new mob_gardien_de_la_nuee_cramoisieAI(pCreature);
    }

};

/******************************
 **Prophète de la Nuée Cramoisie
 *******************************/

#define spell_bouclier_du_crepuscule_flamboyant 90308 // Bouclier_du_Crepuscule_Flamboyant
#define spell_flamme_corrompue 90653 // Flamme_Corrompue

class mob_prophete_de_la_nuee_cramoisie: public CreatureScript
{
public:
    mob_prophete_de_la_nuee_cramoisie() : CreatureScript("mob_prophete_de_la_nuee_cramoisie") {}

    struct mob_prophete_de_la_nuee_cramoisieAI : public ScriptedAI
    {
        mob_prophete_de_la_nuee_cramoisieAI(Creature *c) : ScriptedAI(c) {}

        uint32 bouclier_du_crepuscule_flamboyant;
        uint32 flamme_corrompue;

        void Reset()
        {
            bouclier_du_crepuscule_flamboyant = 30000;
            flamme_corrompue = 10000;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (bouclier_du_crepuscule_flamboyant<= diff)
            {
                DoCast(me, spell_bouclier_du_crepuscule_flamboyant);
                bouclier_du_crepuscule_flamboyant = 30000;
            } else bouclier_du_crepuscule_flamboyant-= diff;

            if (flamme_corrompue<= diff)
            {
                DoCastVictim(spell_flamme_corrompue);
                flamme_corrompue = 10000;
            } else flamme_corrompue-= diff;

            DoMeleeAttackIfReady();
        }

    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new mob_prophete_de_la_nuee_cramoisieAI(pCreature);
    }

};

/*************************
 **Prophète de la Nuée Azur
 **************************/

#define spell_arcanes_distordues 90844 // Arcanes_Distordues
#define spell_crepuscule_deforme 90300 // Crépuscule_Déformé
#define spell_eclair_crepusculaire 90303 // Eclair_Crépusculaire

class mob_prophete_de_la_nuee_azur: public CreatureScript
{
public:
    mob_prophete_de_la_nuee_azur() : CreatureScript("mob_prophete_de_la_nuee_azur") {}

    struct mob_prophete_de_la_nuee_azurAI : public ScriptedAI
    {
        mob_prophete_de_la_nuee_azurAI(Creature *c) : ScriptedAI(c) {}

        uint32 arcanes_distordues;
        uint32 crepuscule_deforme;
        uint32 eclair_crepusculaire;

        void Reset()
        {
            arcanes_distordues = 15000;
            crepuscule_deforme = 25000;
            eclair_crepusculaire = 10000;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (arcanes_distordues<= diff)
            {
                DoCast(me, spell_arcanes_distordues);
                arcanes_distordues = 15000;
            } else arcanes_distordues-= diff;

            if (crepuscule_deforme<= diff)
            {
                DoCastVictim(spell_crepuscule_deforme);
                crepuscule_deforme = 25000;
            } else crepuscule_deforme-= diff;

            if (eclair_crepusculaire<= diff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                    DoCast(pTarget, spell_eclair_crepusculaire);
                eclair_crepusculaire = 10000;
            } else eclair_crepusculaire-= diff;

            DoMeleeAttackIfReady();
        }

    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new mob_prophete_de_la_nuee_azurAI(pCreature);
    }

};

/************************
 **Esprit de l'Eau Asservi
 *************************/

#define spell_trait_deau2 90490 // Trait_d'Eau

class  mob_esprit_de_leau_asservi: public CreatureScript
{
public:
    mob_esprit_de_leau_asservi() : CreatureScript("mob_esprit_de_leau_asservi") {}

    struct mob_esprit_de_leau_asserviAI : public ScriptedAI
    {
        mob_esprit_de_leau_asserviAI(Creature *c) : ScriptedAI(c) {}

        uint32 trait_deau;

        void Reset()
        {
            trait_deau = 3000;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (trait_deau<= diff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                    DoCast(pTarget, spell_trait_deau2);
                trait_deau = 3000;
            } else trait_deau-= diff;

            DoMeleeAttackIfReady();
        }

    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new mob_esprit_de_leau_asserviAI(pCreature);
    }

};

/*****************************
 **Elementaire de Roche Asservi
 ******************************/

#define spell_bouclier_rocheux_dentele 76014 // Bouclier_Rocheux_Dentele

class mob_elementaire_de_roche_asservi: public CreatureScript
{
public:
    mob_elementaire_de_roche_asservi() : CreatureScript("mob_elementaire_de_roche_asservi") {}

    struct mob_elementaire_de_roche_asserviAI : public ScriptedAI
    {
        mob_elementaire_de_roche_asserviAI(Creature *c) : ScriptedAI(c) {}

        uint32 bouclier_rocheux_dentele;

        void Reset()
        {
            bouclier_rocheux_dentele = 15000;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (bouclier_rocheux_dentele<= diff)
            {
                DoCast(me, spell_bouclier_rocheux_dentele);
                bouclier_rocheux_dentele = 15000;
            } else bouclier_rocheux_dentele-= diff;

            DoMeleeAttackIfReady();
        }

    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new mob_elementaire_de_roche_asserviAI(pCreature);
    }

};

/***********************
 **Chercheflamme Rehaussé
 ************************/

#define spell_feu_eruptif 90693 // Feu_Eruptif
#define spell_flammes_deconcertantes 76514 // Flammes_Déconcertantes

class mob_chercheflamme_rehausse: public CreatureScript
{
public:
    mob_chercheflamme_rehausse() : CreatureScript("mob_chercheflamme_rehausse") {}

    struct mob_chercheflamme_rehausseAI : public ScriptedAI
    {
        mob_chercheflamme_rehausseAI(Creature *c) : ScriptedAI(c) {}

        uint32 feu_eruptif;
        uint32 flammes_deconcertantes;

        void Reset()
        {
            feu_eruptif = 25000;
            flammes_deconcertantes = 30000;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (feu_eruptif<= diff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                    DoCast(pTarget, spell_feu_eruptif);
                feu_eruptif = 25000;
            } else feu_eruptif-= diff;

            if (flammes_deconcertantes<= diff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                    DoCast(pTarget, spell_flammes_deconcertantes);
                flammes_deconcertantes = 30000;
            } else flammes_deconcertantes-= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new mob_chercheflamme_rehausseAI(pCreature);
    }

};

/*********************
**Marche-Vent Rehaussé
**********************/

#define spell_marche_vent 76557 // Marche_Vent

class mob_marche_vent_rehausse: public CreatureScript
{
public:
    mob_marche_vent_rehausse() : CreatureScript("mob_marche_vent_rehausse") {}

    struct mob_marche_vent_rehausseAI : public ScriptedAI
    {
        mob_marche_vent_rehausseAI(Creature *c) : ScriptedAI(c) {}

        uint32 marche_vent;

        void Reset()
        {
            marche_vent = 20000;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (marche_vent<= diff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 10.0f, true))
                    DoCast(target, spell_marche_vent);
                marche_vent = 20000;
            } else marche_vent-= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new mob_marche_vent_rehausseAI(pCreature);
    }

};

/*****************************
 **Seigneur de Guerre Nuée Azur
 ******************************/

#define spell_deflagration_azur 90697 // Déflagration_Azur
#define spell_invocation_dun_visage_tordu 90695 // Invocation_d'un_Visage_Tordu

class mob_seigneur_de_guerre_de_la_nuee_azur: public CreatureScript
{
public:
    mob_seigneur_de_guerre_de_la_nuee_azur() : CreatureScript("mob_seigneur_de_guerre_de_la_nuee_azur") {}

    struct mob_seigneur_de_guerre_de_la_nuee_azurAI : public ScriptedAI
    {
        mob_seigneur_de_guerre_de_la_nuee_azurAI(Creature *c) : ScriptedAI(c) {}

        uint32 deflagration_azur;
        uint32 invocation_dun_visage_tordu;

        void Reset()
        {
            deflagration_azur = 30000;
            invocation_dun_visage_tordu = 45000;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (deflagration_azur<= diff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, -10.0f, true))
                    DoCast(target, spell_deflagration_azur);
                deflagration_azur = 30000;
            } else deflagration_azur-= diff;

            if (invocation_dun_visage_tordu<= diff)
            {
                DoCast(spell_invocation_dun_visage_tordu);
                invocation_dun_visage_tordu = 45000;
            } else invocation_dun_visage_tordu-= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new mob_seigneur_de_guerre_de_la_nuee_azurAI(pCreature);
    }

};

/****************************************
 **Seigneur de Guerre de la Nuee Cramoisie
 *****************************************/

#define spell_brasier_desarmant 90705 // Brasier_Desarmant
#define spell_renforcement_du_crepuscule 90704 // Renforcement_du_Crepuscule

class mob_seigneur_de_guerre_de_la_nuee_cramoisie: public CreatureScript
{
public:
    mob_seigneur_de_guerre_de_la_nuee_cramoisie() : CreatureScript("mob_seigneur_de_guerre_de_la_nuee_cramoisie") {}

    struct mob_seigneur_de_guerre_de_la_nuee_cramoisieAI : public ScriptedAI
    {
        mob_seigneur_de_guerre_de_la_nuee_cramoisieAI(Creature *c) : ScriptedAI(c) {}

        uint32 brasier_desarmant;
        uint32 renforcement_du_crepuscule;

        void Reset()
        {
            brasier_desarmant = 15000;
            renforcement_du_crepuscule = 25000;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (brasier_desarmant<= diff)
            {
                DoCastVictim(spell_brasier_desarmant);
                brasier_desarmant = 15000;
            } else brasier_desarmant-= diff;

            if (renforcement_du_crepuscule<= diff)
            {
                DoCast(me, spell_renforcement_du_crepuscule);
                renforcement_du_crepuscule = 25000;
            } else renforcement_du_crepuscule-= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new mob_seigneur_de_guerre_de_la_nuee_cramoisieAI(pCreature);
    }
};

/************************
 **Imposteur du Crépuscule
 *************************/

#define spell_duperie 90719 // Duperie
#define spell_esprit_enchaine 90715 // Esprit_Enchainé
#define spell_explosion_trompeuse 90712 // Explosion_Trompeuse

class mob_imposteur_du_crepuscule: public CreatureScript
{
public:
    mob_imposteur_du_crepuscule() : CreatureScript("mob_imposteur_du_crepuscule") {}

    struct mob_imposteur_du_crepusculeAI : public ScriptedAI
    {
        mob_imposteur_du_crepusculeAI(Creature *c) : ScriptedAI(c) {}

        uint32 duperie;
        uint32 esprit_enchaine;
        uint32 explosion_trompeuse;

        void Reset()
        {
            duperie = 45000;
            esprit_enchaine = 17000;
            explosion_trompeuse = 13000;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (duperie<= diff)
            {
                DoCast(SelectTarget(SELECT_TARGET_RANDOM,1,100,true), spell_duperie);
                duperie = 45000;
            } else duperie-= diff;

            if (esprit_enchaine<= diff)
            {
                DoCast(SelectTarget(SELECT_TARGET_RANDOM,1,100,true), spell_esprit_enchaine);
                esprit_enchaine = 17000;
            } else esprit_enchaine-= diff;

            if (explosion_trompeuse<= diff)
            {
                DoCast(spell_explosion_trompeuse);
                explosion_trompeuse = 13000;
            } else explosion_trompeuse-= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new mob_imposteur_du_crepusculeAI(pCreature);
    }

};

/*********************
 **Brute Gronn Asservie
 **********************/

#define spell_broie_armure 90710 // Broie_Armure

class mob_brute_gronn_asservie: public CreatureScript
{
public:
    mob_brute_gronn_asservie() : CreatureScript("mob_brute_gronn_asservie") {}

    struct mob_brute_gronn_asservieAI : public ScriptedAI
    {
        mob_brute_gronn_asservieAI(Creature *c) : ScriptedAI(c) {}

        uint32 broie_armure;

        void Reset()
        {
            broie_armure = 20000;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (broie_armure<= diff)
            {
                DoCastVictim(spell_broie_armure);
                broie_armure = 20000;
            } else broie_armure-= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new mob_brute_gronn_asservieAI(pCreature);
    }

};

/****************************
 **Esprit du Tonnerre Asservie
 *****************************/

#define spell_appel_declair 90852 // Appel_d'Eclair

class mob_esprit_du_tonnerre_asservie: public CreatureScript
{
public:
    mob_esprit_du_tonnerre_asservie() : CreatureScript("mob_esprit_du_tonnerre_asservie") {}

    struct mob_esprit_du_tonnerre_asservieAI : public ScriptedAI
    {
        mob_esprit_du_tonnerre_asservieAI(Creature *c) : ScriptedAI(c) {}

        uint32 appel_declair;

        void Reset()
        {
            appel_declair = 15000;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (appel_declair<= diff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                    DoCast(pTarget, spell_appel_declair);
                appel_declair = 15000;
            } else appel_declair-= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new mob_esprit_du_tonnerre_asservieAI(pCreature);
    }

};

/*****************************
 **Mande Tonnerre du Crépuscule
 ******************************/

#define spell_chaine_declairs 90856 // Chaines_D'éclairs
#define spell_invocation_dun_esprit_du_tonnerre 75096 // Invocation_d'un_Esprit_du_Tonnerre
#define spell_surcharger 90858 // Surcharger

class mob_mande_tonnerre_du_crepuscule: public CreatureScript
{
public:
    mob_mande_tonnerre_du_crepuscule() : CreatureScript("mob_mande_tonnerre_du_crepuscule") {}

    struct mob_mande_tonnerre_du_crepusculeAI : public ScriptedAI
    {
        mob_mande_tonnerre_du_crepusculeAI(Creature *c) : ScriptedAI(c) {}

        uint32 chaine_declairs;
        uint32 invocation_dun_esprit_du_tonnerre;
        uint32 surcharger;

        void Reset()
        {
            chaine_declairs = 15000;
            invocation_dun_esprit_du_tonnerre = 50000;
            surcharger = 6000;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (chaine_declairs<= diff)
            {
                DoCast(SelectTarget(SELECT_TARGET_RANDOM,1,100,true), spell_chaine_declairs);
                chaine_declairs = 15000;
            } else chaine_declairs-= diff;

            if (invocation_dun_esprit_du_tonnerre<= diff)
            {
                DoCast(spell_invocation_dun_esprit_du_tonnerre);
                invocation_dun_esprit_du_tonnerre = 50000;
            } else invocation_dun_esprit_du_tonnerre-= diff;

            if (surcharger<= diff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 10.0f, true))
                    DoCast(target, spell_surcharger);
                surcharger = 6000;
            } else surcharger-= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new mob_mande_tonnerre_du_crepusculeAI(pCreature);
    }

};

/************************
 **Braise Ardente Asservie
 *************************/

#define spell_horion_de_flammes 90846 // Horion_de_Flammes

class mob_braise_ardente_asservie: public CreatureScript
{
public:
    mob_braise_ardente_asservie() : CreatureScript("mob_braise_ardente_asservie") {}

    struct mob_braise_ardente_asservieAI : public ScriptedAI
    {
        mob_braise_ardente_asservieAI(Creature *c) : ScriptedAI(c) {}

        uint32 horion_de_flammes;

        void Reset()
        {
            horion_de_flammes = 20000;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (horion_de_flammes<= diff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                    DoCast(pTarget, spell_horion_de_flammes);
                horion_de_flammes = 20000;
            } else horion_de_flammes-= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new mob_braise_ardente_asservieAI(pCreature);
    }

};

/******************************
 **Flagellant Aquatique Rehaussé
 *******************************/

#define spell_geyser_focalise 90870 // Geyser_Focalisé
#define spell_jet_deau 90868 // Jet_d'Eau
#define spell_nuage_declairs 76097 // Nuage_d'Eclairs

class mob_flagellant_aquatique_rehausse: public CreatureScript
{
public:
    mob_flagellant_aquatique_rehausse() : CreatureScript("mob_flagellant_aquatique_rehausse") {}

    struct mob_flagellant_aquatique_rehausseAI : public ScriptedAI
    {
        mob_flagellant_aquatique_rehausseAI(Creature *c) : ScriptedAI(c) {}

        uint32 geyser_focalise;
        uint32 jet_deau;
        uint32 nuage_declairs;

        void Reset()
        {
            geyser_focalise = 30000;
            jet_deau = 15000;
            nuage_declairs = 23000;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (geyser_focalise<= diff)
            {
                DoCastVictim(spell_geyser_focalise);
                geyser_focalise = 30000;
            } else geyser_focalise-= diff;

            if (jet_deau<= diff)
            {
                DoCast(SelectTarget(SELECT_TARGET_RANDOM,1,100,true), spell_jet_deau);
                jet_deau = 15000;
            } else jet_deau-= diff;

            if (nuage_declairs<= diff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 10.0f, true))
                    DoCast(target, spell_nuage_declairs);
                nuage_declairs = 23000;
            } else nuage_declairs-= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new mob_flagellant_aquatique_rehausseAI(pCreature);
    }

};

/*********************
 **Brise-Roche Rehaussé
 **********************/

#define spell_blessure_brulante 76093 // Blessure_Brulante
#define spell_choc_de_pierre 90866 // Choc_de_Pierre
#define spell_poings_ardents 76086 // Poings_Ardents

class mob_brise_roche_rehausse: public CreatureScript
{
public:
    mob_brise_roche_rehausse() : CreatureScript("mob_brise_roche_rehausse") {}

    struct mob_brise_roche_rehausseAI : ScriptedAI
    {
        mob_brise_roche_rehausseAI(Creature *c) : ScriptedAI(c) {}

        uint32 blessure_brulante;
        uint32 choc_de_pierre;
        uint32 poings_ardents;

        void Reset()
        {
            blessure_brulante = 15000;
            choc_de_pierre = 20000;
            poings_ardents = 35000;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (blessure_brulante<= diff)
            {
                DoCastVictim(spell_blessure_brulante);
                blessure_brulante = 15000;
            } else blessure_brulante-= diff;

            if (choc_de_pierre<= diff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 10.0f, true))
                    DoCast(me, spell_choc_de_pierre);
                choc_de_pierre = 20000;
            } else choc_de_pierre-= diff;

            if (poings_ardents<= diff)
            {
                DoCast(me, spell_poings_ardents);
                poings_ardents = 35000;
            } else poings_ardents-= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new mob_brise_roche_rehausseAI(pCreature);
    }

};

/********************
 **Drake du Crepuscule
 *********************/

#define spell_flammes_du_crepuscule 75931 // Flammes_du_Crépuscule
#define spell_souffle_crepusculaire 76817 // Souffle_Crepusculaire

class mob_drake_du_crepuscule: public CreatureScript
{
public:
    mob_drake_du_crepuscule() : CreatureScript("mob_drake_du_crepuscule") {}

    struct mob_drake_du_crepusculeAI : public ScriptedAI
    {
        mob_drake_du_crepusculeAI(Creature *c) : ScriptedAI(c) {}

        uint32 flammes_du_crepuscule;
        uint32 souffle_crepusculaire;

        void Reset()
        {
            flammes_du_crepuscule = 25000;
            souffle_crepusculaire = 15000;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (flammes_du_crepuscule<= diff)
            {
                DoCast(SelectTarget(SELECT_TARGET_RANDOM,1,100,true), spell_flammes_du_crepuscule);
                flammes_du_crepuscule = 25000;
            } else flammes_du_crepuscule-= diff;

            if (souffle_crepusculaire<= diff)
            {
                DoCastVictim(spell_souffle_crepusculaire);
                souffle_crepusculaire = 25000;
            } else souffle_crepusculaire-= diff;

            DoMeleeAttackIfReady();
        }
    };



    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new mob_drake_du_crepusculeAI(pCreature);
    }

};

/***********************
 **Corrupteur Sans-Visage
 ************************/

#define spell_guerison_umbrale 91040 // Guerison_Umbrale
#define spell_siphon_dessence 91028 // Siphon_d'Essence

class mob_corrupteur_sans_visage: public CreatureScript
{
public:
    mob_corrupteur_sans_visage() : CreatureScript("mob_corrupteur_sans_visage") {}

    struct mob_corrupteur_sans_visageAI : public ScriptedAI
    {
        mob_corrupteur_sans_visageAI(Creature *c) : ScriptedAI(c) {}

        uint32 guerison_umbrale;
        uint32 siphon_dessence;

        void Reset()
        {
            guerison_umbrale = 20000;
            siphon_dessence = 15000;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (guerison_umbrale<= diff)
            {
                DoCast(me, spell_guerison_umbrale);
                guerison_umbrale = 20000;
            } else guerison_umbrale-= diff;

            if (siphon_dessence<= diff)
            {
                if (Unit *pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                    DoCast(pTarget, spell_siphon_dessence);
                siphon_dessence = 15000;
            } else siphon_dessence-= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new mob_corrupteur_sans_visageAI(pCreature);
    }

};

class spell_linceul_dummy : public SpellScriptLoader
{
public:
    spell_linceul_dummy() : SpellScriptLoader("spell_linceul_dummy") { }

    class spell_linceul_dummy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_linceul_dummy_SpellScript);

        void HandleDummyHitTarget(SpellEffIndex /*effIndex*/)
        {
            if (Unit* caster = GetCaster())
                if (Vehicle *veh = caster->GetVehicleKit())
                    if (Unit *player = veh->GetPassenger(0))
                        if (Unit* targ = GetHitUnit())
                            if (Creature *target = targ->ToCreature())
                            {
                                caster->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE|UNIT_FLAG_NOT_SELECTABLE);
                                target->AddThreat(player, 1.0f);
                                target->SetInCombatWith(player);
                                target->AI()->AttackStart(player);
                            }
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_linceul_dummy_SpellScript::HandleDummyHitTarget, EFFECT_1, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_linceul_dummy_SpellScript();
    }
};


void AddSC_mob_trash_grim_batol()
{
    new mob_gardien_de_la_nuee_azur();
    new mob_gardien_de_la_nuee_cramoisie();
    new mob_prophete_de_la_nuee_azur();
    new mob_esprit_de_leau_asservi();
    new mob_elementaire_de_roche_asservi();
    new mob_chercheflamme_rehausse();
    new mob_marche_vent_rehausse();
    new mob_seigneur_de_guerre_de_la_nuee_azur();
    new mob_seigneur_de_guerre_de_la_nuee_cramoisie();
    new mob_imposteur_du_crepuscule();
    new mob_brute_gronn_asservie();
    new mob_esprit_du_tonnerre_asservie();
    new mob_mande_tonnerre_du_crepuscule();
    new mob_braise_ardente_asservie();
    new mob_flagellant_aquatique_rehausse();
    new mob_brise_roche_rehausse();
    new mob_drake_du_crepuscule();
    new mob_corrupteur_sans_visage();
    new spell_linceul_dummy();
}
