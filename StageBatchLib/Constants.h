/*##########################################################################
//
// AVIS IMPORTANT: ce logiciel, incluant le code source, contient de l’information
// confidentielle et/ou propriété de Presagis Canada inc. et/ou ses filiales et
// compagnies affiliées (« Presagis ») et est protégé par les lois sur le droit
// d’auteur. L’utilisation de ce logiciel est sujet aux termes d’une licence de
// Presagis.
// Malgré toute disposition contraire, l'utilisateur titulaire
// d'une licence peut utiliser les exemples de code pour générer de nouvelles
// applications.
// © Presagis Canada Inc., 1995-2011. Tous droits réservés.
//
// IMPORTANT NOTICE:  This software program (“Software”) and/or  the Source
// code is a confidential and/or proprietary product of Presagis Canada Inc.
// and/or its subsidiaries and affiliated companies (“Presagis”) and is
// protected by copyright laws.  The use of the Software is subject to the
// terms of the Presagis Software License Agreement.
// Notwithstanding anything to the contrary, the
// licensed end-user is allowed to use the sample source provided with the
// Software to generate new applications.
// © Presagis Canada Inc., 1995-2011. All Rights Reserved.
//
*###########################################################################*/

#ifndef STAGE_BATCH_LIB_CONSTANTS_H
#define STAGE_BATCH_LIB_CONSTANTS_H

//=============================================================================
// C O N S T A N T S
//=============================================================================

/* Name of the SIM batch file */
#define SIM_CMD_FILE "stage.sim_cmd"
#define SERVER_CONNECTION "EXECUTABLES\\Server.dat"
#define STAGE_PUBLISHER "RunMainPub.cmd"
#define STAGE_SUBSCRIBER "RunMainSub.cmd"
#define TEMP_XML_BUFFER_FILE "tempFile.xml"
#define OUTPUT_XML_FILE "OutputFile.xml"

#define SIM_MISSION_FILE "entitymission.missionactive"

#define BUF_SIZE 256

#define STOP_CONDITION_TIME_ELAPSED "Stop_At_Time"
#define STOP_CONDITION_ENTITY_DEAD "Stop_When_Ent_Killed"
#define STOP_CONDITION_MIN_RED_PLATFORM_COUNT "Stop_When_Red_Entity_Count_Under"
#define STOP_CONDITION_MIN_BLUE_PLATFORM_COUNT "Stop_When_Blue_Entity_Count_Under"

#endif
