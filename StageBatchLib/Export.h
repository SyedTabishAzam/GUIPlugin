/*##########################################################################
//
// AVIS IMPORTANT: ce logiciel, incluant le code source, contient de l�information
// confidentielle et/ou propri�t� de Presagis Canada inc. et/ou ses filiales et
// compagnies affili�es (� Presagis �) et est prot�g� par les lois sur le droit
// d�auteur. L�utilisation de ce logiciel est sujet aux termes d�une licence de
// Presagis.
// Malgr� toute disposition contraire, l'utilisateur titulaire
// d'une licence peut utiliser les exemples de code pour g�n�rer de nouvelles
// applications.
// � Presagis Canada Inc., 1995-2011. Tous droits r�serv�s.
//
// IMPORTANT NOTICE:  This software program (�Software�) and/or  the Source
// code is a confidential and/or proprietary product of Presagis Canada Inc.
// and/or its subsidiaries and affiliated companies (�Presagis�) and is
// protected by copyright laws.  The use of the Software is subject to the
// terms of the Presagis Software License Agreement.
// Notwithstanding anything to the contrary, the
// licensed end-user is allowed to use the sample source provided with the
// Software to generate new applications.
// � Presagis Canada Inc., 1995-2011. All Rights Reserved.
//
*###########################################################################*/

#ifndef STAGE_BATCH_LIB_EXPORT_H
#define STAGE_BATCH_LIB_EXPORT_H

#ifdef WIN32
   #ifdef STAGE_BATCH_LIB_DLL
      #define STAGE_BATCH_LIB_SPEC __declspec(dllexport)
   #else
      #define STAGE_BATCH_LIB_SPEC __declspec(dllimport)
   #endif
#else
   #define STAGE_BATCH_LIB_SPEC
#endif

#endif
  
