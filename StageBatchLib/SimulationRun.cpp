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

//=============================================================================
// I N C L U D E S
//=============================================================================
#include "SimulationRun.h"

//=============================================================================
// C O N S T R U C T O R S   A N D   D E S T R U C T O R S
//=============================================================================
SimulationRun::SimulationRun()
   : Name(L"Unnamed"), IsSnapshotRestore(SQX_FALSE), IterationCount(1),
   DelayInSeconds(0), Seed(0),
   stopConditions(StopConditions_None), StopTimeInSeconds(0),
   MinRedPlatformCount(0), MinBluePlatformCount(0)
{
}
