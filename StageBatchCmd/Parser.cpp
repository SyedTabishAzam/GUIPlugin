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
#include "Parser.h"

#include "Constants.h"
#include "SimulationRun.h"

#include <stagePath.h>

#include <sqxOSSystem.h>
#include <sqxOSSystemW.h>

#include <sig_function.h>
#include <sig_mdep.h>
#include <sig_path.h>
#include <sig_print.h>

#include <cstdio>

using namespace std;

//=============================================================================
// L O C A L   C O N S T A N T S
//=============================================================================
static const sqxChar s_CommentChar = '#';
static const sqxChar s_SeparatorChar = ':';

static const sqxChar* s_pRunKeyword = "RUN";
static const sqxChar* s_pRestoreKeyword = "RESTORE";

static const sqxChar* s_pScenarioKeyword = "Scenario";
static const sqxChar* s_pDelayKeyword = "Delay";
static const sqxChar* s_pSnapfileKeyword = "SnapFile";
static const sqxChar* s_pSnapshotKeyword = "Snapshot";
static const sqxChar* s_pStopKeyword = "Stop_Cond";
static const sqxChar* s_pSeedKeyword = "Seed";

//=============================================================================
// L O C A L   T Y P E S
//=============================================================================
/// @brief Flags indicating the state of the parsing process.
enum ParsingFlags
{
   /// @brief Initial parsing state
   ParsingFlags_None = 0,

   /// @brief RUN command encountered, parameters being parsed
   ParsingFlags_RunParsed = 1 << 0,

   /// @brief SCENARIO parameter parsed (implies ParsingFlags_RunParsed)
   ParsingFlags_ScenarioParsed = 1 << 1,

   /// @brief DELAY encountered
   ParsingFlags_DelayParsed = 1 << 2,

   /// @brief SNAPFILE parameter parsed (implies ParsingFlags_RestoreParsed)
   ParsingFlags_SnapFileParsed = 1 << 5,

   /// @brief RESTORE command encountered, parameters being parsed
   ParsingFlags_RestoreParsed = 1 << 6,

   /// @brief All parameters for the run command parsed
   ParsingFlags_AllRun = ParsingFlags_RunParsed
      | ParsingFlags_ScenarioParsed | ParsingFlags_DelayParsed,

   /// @brief All parameters for the restore command parsed
   ParsingFlags_AllRestore = ParsingFlags_SnapFileParsed
      | ParsingFlags_RestoreParsed | ParsingFlags_DelayParsed
};

/// @brief Tests if a given flags value has the flags in the specified mask.
static sqxBool HasFlags(ParsingFlags a_Value, ParsingFlags a_Mask)
{
   return (a_Value & a_Mask) == a_Mask;
}

/// @brief Combines a mask into a flags value.
inline ParsingFlags& operator |=(ParsingFlags& a_Value, ParsingFlags a_Mask)
{
   a_Value = static_cast<ParsingFlags>(a_Value | a_Mask);
   return a_Value;
}

//=============================================================================
// L O C A L   F U N C T I O N S
//=============================================================================
static SimulationRun* CreateSimulationRun()
{
   SimulationRun* _pRun = new SimulationRun();

   const sqxWChar* _pStageDir = stage_get_path(STAGE_DIR);
   if (_pStageDir == SQX_NULL) _pStageDir = L"C:";
   _pRun->DatabaseFilePath = wstring(_pStageDir)
      + g_pOSSystemW->GetPathSeparator() + L"GUIPlugin"  
      + g_pOSSystemW->GetPathSeparator() + L"batch.xml";

   g_pOSSystemW->NormalizePath(_pRun->DatabaseFilePath);
   return _pRun;
}

//=============================================================================
// E X P O R T E D   F U N C T I O N S
//=============================================================================
sqxBool ParseBatchFile(const sqxChar* a_pFilePath, std::vector<SimulationRun*>& a_rRuns)
{
   stage_path_init();

   // Open batch file
   FILE* _pFile = fopen(a_pFilePath, "r");
   if (_pFile == NULL)
   {
      sig_print("%s:%d: Error: Cannot open file '%s' for parsing !\n",
                __FILE__, __LINE__, a_pFilePath);
      return false;
   }
   else
   {
      sig_print("Parsing STAGE command file '%s'...\n", a_pFilePath);
   }

   int _LineNumber = 0;
   const char* _pErrorMessage = SQX_NULL;

   ParsingFlags _Flags = ParsingFlags_None;
   SimulationRun* _pRun = SQX_NULL;

   sqxChar buffer[BUF_SIZE + 1];
   sqxChar buf_copy[BUF_SIZE + 1];
   
   for (;;)
   {   
      // Read a line from the batch file
      fgets(buffer, BUF_SIZE, _pFile);
      
      if (feof(_pFile))
         break;
      
      _LineNumber++;
      
      // Check if it is a comment or empty line
      if (buffer[0] == s_CommentChar || strlen(buffer) == 0)
         continue;
      
      // Keep a copy of the buffer as strtok will write into the original...
      strcpy(buf_copy, buffer);
      
      // Check if line is blank
      const char* _pKeyword = strtok(buffer," \t\n");
      if (_pKeyword == NULL)
         continue;
      
      if (!strcmp(_pKeyword, s_pRunKeyword))
      {
         // Check if line order is OK
         if (_Flags != 0 && _Flags != ParsingFlags_AllRun && _Flags != ParsingFlags_AllRestore)
         {
            _pErrorMessage = "RUN command encountered while parsing another command";
            break;
         }

         _Flags = ParsingFlags_RunParsed;
        
         // Create a new run
         _pRun = CreateSimulationRun();
         a_rRuns.push_back(_pRun);
         
         // Retrieve the run name
         const sqxChar* _pToken = strtok(NULL, ": \t\n");
         if (_pToken != SQX_NULL && _pToken[0] != '\0')
         {
            // Store the run description name
            const char* _pName = buf_copy + (_pToken - buffer);
            std::string _Name(_pName, strlen(_pName) - 1);
            _pRun->Name.assign(g_pOSSystem->AnsiToUnicode(_Name));
         }
         else
         {	        
            _pRun->Name.clear();
         }

         continue;
      }
      else if (!strcmp(_pKeyword, s_pRestoreKeyword))
      {
         // Check if line order is OK
         if (_Flags != 0 && _Flags != ParsingFlags_AllRun && _Flags != ParsingFlags_AllRestore)
         {
            _pErrorMessage = "RESTORE command encountered while parsing another command";
            break;
         }

         _Flags = ParsingFlags_RestoreParsed;
         
         // Create a new run
         _pRun = CreateSimulationRun();
         a_rRuns.push_back(_pRun);

         // Retrieve the restore name
         const sqxChar* _pToken = strtok(NULL, ": \t\n");
         if (_pToken != SQX_NULL && _pToken[0] != '\0')
         {
            // Store the run description name
            const char* _pName = buf_copy + (_pToken - buffer);
            std::string _Name(_pName, strlen(_pName) - 1);
            _pRun->Name.assign(g_pOSSystem->AnsiToUnicode(_Name));
         }
         else
         {	        
            _pRun->Name.clear();
         }

         continue;
      }
      else if (!strcmp(_pKeyword, s_pScenarioKeyword))
      {
         if (!HasFlags(_Flags, ParsingFlags_RunParsed))
         {
            _pErrorMessage = "SCENARIO parameter encountered when not parsing a RUN command";
            break;
         }   

         

         // Retrieve the scenario name
         const sqxChar* _pToken = strtok(NULL, ": \t\n");
         if (_pToken != SQX_NULL && _pToken[0] != '\0')
         {
            // Store the scenario name
            std::string _FilePath(_pToken);
            _pRun->ScenarioFilePath = g_pOSSystem->AnsiToUnicode(_FilePath);
         }
         else
         {	        
            // Scenario name can be missing (use current loaded scenario)
            _pRun->ScenarioFilePath.clear();
         }

         _Flags |= ParsingFlags_ScenarioParsed;
         continue;
      }
      else if (!strcmp(_pKeyword, s_pSnapfileKeyword))
      {
         if (!HasFlags(_Flags, ParsingFlags_RestoreParsed))
         {
            _pErrorMessage = "SNAPFILE parameter encountered when not parsing a RESTORE command";
            break;
         }   

         

         // Retrieve the snapshot filename
         const sqxChar* _pFilePath = strtok(NULL, ": \t\n");
         if (_pFilePath != SQX_NULL && _pFilePath[0] != '\0')
         {
            if (sig_is_basename(_pFilePath))
            {
               // Build the full pathname
               _pRun->ScenarioFilePath = stage_get_path(STAGE_RECORDING_DIR);
               _pRun->ScenarioFilePath += SIG_PATH_SEPW;
               _pRun->ScenarioFilePath += g_pOSSystem->AnsiToUnicode(_pFilePath);
            }
            else
            {
               // Store the filename
               _pRun->ScenarioFilePath = g_pOSSystem->AnsiToUnicode(_pFilePath);
            }
         }

         _Flags |= ParsingFlags_SnapFileParsed;
         continue;
      }
      else if (!strcmp(_pKeyword, s_pStopKeyword))
      {
         // Retrieve the stop condition
         const sqxChar* _pCondition = strtok(NULL, ": \t\n");
         if (_pCondition != SQX_NULL && _pCondition[0] != '\0')
         {
            if (!strcmp(_pCondition, STOP_CONDITION_TIME_ELAPSED))
            {
               const sqxChar* _pTimeString = strtok(NULL, " \t\n");
               if (_pTimeString != SQX_NULL && _pTimeString[0] != '\0')
               {
                  float _Time;
                  if (sig_str_to_time(&_Time, const_cast<char*>(_pTimeString)) == 0)
                  {
                     _pRun->stopConditions |= StopConditions_ElapsedTime;
                     _pRun->StopTimeInSeconds = _Time;
                  }
               }
            }
            else if(!strcmp(_pCondition, STOP_CONDITION_ENTITY_DEAD))
            {
               // TODO: Implement support for this in sim code
               const sqxChar* _pEntityName = strtok(NULL, ": \t\n");
               if (_pEntityName != SQX_NULL && _pEntityName[0] != '\0')
               {
                  _pRun->stopConditions |= StopConditions_DeadEntity;
                  
				  
               }
            }
            else
            {
               sig_print("Unknown stop condition: %s\n", _pCondition);
            }
         }
      }
     
      
     
      else
      {
         // SIM command lines
         if (!HasFlags(_Flags, ParsingFlags_RunParsed)
            && !HasFlags(_Flags, ParsingFlags_RestoreParsed))
         {
            _pErrorMessage = "Sim command line encountered when not parsing a command";
            break;
         }

         std::string _Param = buf_copy;

         // Remove the : in the command string
         size_t _Index = _Param.find(s_SeparatorChar);
         if (_Index != std::string::npos) _Param[_Index] = ' ';

         // Store the whole SIM command line
         _pRun->SimParams.push_back(_Param);
      }
   }
   
   // Check if line order is OK
   if (_Flags != 0 && _Flags != ParsingFlags_AllRun && _Flags != ParsingFlags_AllRestore)
      _pErrorMessage = "Empty or incomplete command file";
   
   fclose(_pFile);
   
   if (_pErrorMessage)
   {
      sig_print(
         "%s:%d: Parse error: %s\n"
         "In file '%s' at line %d: %s\n",
         __FILE__, __LINE__, _pErrorMessage,
         a_pFilePath, _LineNumber, buf_copy);
      return false;
   }

   return true;
}
