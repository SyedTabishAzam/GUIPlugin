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

#ifndef STAGE_BATCH_CMD_PARSER_H
#define STAGE_BATCH_CMD_PARSER_H

//=============================================================================
// I N C L U D E S
//=============================================================================
#include "SimulationRun.h"

#include "sqxTypes.h"

#include <vector>

//=============================================================================
// E X P O R T E D   F U N C T I O N S
//=============================================================================
/// @brief Parses a batch file and returns the simulation runs it defines.
///
/// @param a_pFilePath A path to the commands file.
/// @param a_rRuns A vector of simulation runs where parsed runs are returned.
sqxBool ParseBatchFile(const sqxChar* a_pFilePath, std::vector<SimulationRun*>& a_rRuns);

#endif
