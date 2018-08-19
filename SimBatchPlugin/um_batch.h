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

/***************************************************************/
/* I N C L U D E   F I L E   P U R P O S E                     */
/*                                                             */
/*[ Function prototyping of the external functions coded in    */
/*  um_ext_rts.c                                              ]*/
/***************************************************************/

#ifndef UM_BATCH_H
#define UM_BATCH_H

/***************************************************************/
/* I N C L U D E D   F I L E S                                 */
/***************************************************************/
#include "Constants.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sig_print.h>

#include <stage_entity.h>
#include <msg/stage_msg_exp.h>
#include <sim/sim_rtc.h>
#include <sim/sim_ptf.h>
#include <sim/sim_weapon.h>
#include <sim/sim_ent_model.h>
#include <sim/sim_mission.h>

#include <sim/sim_task.h>
#include <sig_str_conv_utils.h>
#include <sig_random.h>
/***************************************************************/
/* C O N S T A N T S                                           */
/***************************************************************/

/***************************************************************/
/* M A C R O S                                                 */
/***************************************************************/

/***************************************************************/
/* D A T A  S T R U C T U R E S                                */
/***************************************************************/

/***************************************************************/
/* E X T E R N A L   F U N C T I O N S                         */
/***************************************************************/

int um_batch_init(void* userdata);
void um_batch_ctrl(int ctrl_type);
void um_batch_exec();
void um_batch_grab_data(void);
void um_tmpl_daemon();
void um_tmpl_background();
/***************************************************************/
/* E X T E R N A L   D A T A                                   */
/***************************************************************/

#endif /* UM_EXT_RTS_H */


