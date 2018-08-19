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
/* M O D U L E    P U R P O S E                                */
/*							                                   */
/*[ This module executes commands from a batch file       .]   */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* I N C L U D E D   F I L E S                                 */
/***************************************************************/
#include <cstring>
#include <atlstr.h>
#include "um_batch.h"
#include <algorithm>
#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <codecvt>
#include <fstream>
#include <stdio.h>
#include <map>
#include <iostream>
#include <stdlib.h>  
#include <string>  

#include <locale>
#include <sstream>

#include <vector>
#include <dirent.h>
//#import <MSXML2.dll> rename_namespace(_T("MSXML"))

#include "tinyxml2.h"

using namespace tinyxml2;

using namespace std;

/***************************************************************/
/* L O C A L   C O N S T A N T S                               */
/***************************************************************/
enum MODE { APPEND, WRITE };

/***************************************************************/
/* L O C A L   M A C R O S                                     */
/***************************************************************/
#ifndef XMLCheckResult
#define XMLCheckResult(a_eResult) if (a_eResult != XML_SUCCESS) {sig_print("Error: %i\n",a_eResult); }
#endif
/***************************************************************/
/* L O C A L   D A T A    S T R U C T U R E S                  */
/***************************************************************/
struct Commands
{
	public:
		string entityName,variableName,value;
};
vector<Commands> CommandQueue;
/***************************************************************/
/* E X T E R N A L    F U N C T I O N S                        */
/***************************************************************/

/***************************************************************/
/* L O C A L    F U N C T I O N S                              */
/***************************************************************/

static int find_config_arg(char *cur_arg);
void HandleData(vector<string>&,  vector<wchar_t*>&);
static void parse_config_file(void);
string WcharToString(wchar_t* wchr);
void StringToWchar(std::string str, wchar_t*& wchr);
int UpdateVariableValue(simValue*&, string);
void SaveDataForPublish();
void UpdateDataForPublish();
BOOL PublishData(DWORD & exitCode);
void CreateXML();
//void ReadXML();
/***************************************************************/
/* E X T E R N A L   D A T A                                   */
/***************************************************************/

/***************************************************************/
/* L O C A L   D A T A                                         */
/***************************************************************/

/* Matches for the keywords in the batch
file. Add any new keywords here */
static char *tab_config_args[] =
{
	"set_ptf0_req_speed", /* 0 */
	"set_ptf1_req_alt",   /* 1 */
	"get_ptf0_nr_msl",    /* 2 */
	"get_ptf1_nr_msl",    /* 3 */
	"SEED",               /* 4 Set random generator seed */
	"RUN",                /* 5 Set run number */
	"ITERATION",          /* 6 Set execution number */
	STOP_CONDITION_TIME_ELAPSED, /* 7 Stop at time condition */
	STOP_CONDITION_ENTITY_DEAD,   /* 8 Stop when specific entity dies */
	/* 9 Stop when red entity count drops below */
	STOP_CONDITION_MIN_RED_PLATFORM_COUNT,
	/* 10 Stop when blue entity count drops below */
	STOP_CONDITION_MIN_BLUE_PLATFORM_COUNT,
};

static Sim_Global_Data *gdp; /* Pointer to the SIM global data */
static Private_Data *prd;    /* Pointer to the SIM private data */
static FILE *out_file = NULL;
static long seed = 0;
static int   *rbidx_ptr;
static double stop_time = -1;
static wchar_t stop_entity_name[GSIM_SCN_PTF_SCN_PTF_NAME_LENGTH + 1] = { L'\0' };
static int min_red_platform_count = 0;
static int min_blue_platform_count = 0;
static int stopped = TRUE;
static int run_number = 0;
static int iteration_number = 0;
static int checkonthis = 0;

static wchar_t mission_entity_name[GSIM_SCN_PTF_SCN_PTF_NAME_LENGTH + 1] = { L'\0' };
bool init = false;
int initialEntities = 0;
const bool DEBUG = false;
/***************************************************************/
/***************************************************************/
/* L O C A L   F U N C T I O N S   C O D E   S E C T I O N     */
/***************************************************************/
/***************************************************************/

template <typename T>
T ConvertString(string &data,int & errorCode)
{
	errorCode = 1;
	if (!data.empty())
	{

		T ret;
		std::istringstream iss(data);
		if (data.find("0x") != std::string::npos)
		{
			iss >> std::hex >> ret;
		}
		else
		{
			iss >> std::dec >> ret;
		}

		if (iss.fail())
		{
			sig_print("Error in type conversion");
			errorCode = 2;
			return T();
			
		}
		errorCode = 0;
		return ret;
	}
	
	return T();
}

void WriteXML(vector<string> entityList, vector<map<string, string>> varListList)
{
	tinyxml2::XMLDocument xmlDoc;

	XMLNode * pRoot = xmlDoc.NewElement("Associated");
	xmlDoc.InsertFirstChild(pRoot);


	vector<string>::iterator it;

	int counter = 0;
	for (it = entityList.begin(); it != entityList.end(); it++)
	{
		string entityName = *it;

		//Associated->EntityName->Variable->Name+Type
		XMLElement * entityXML = xmlDoc.NewElement(entityName.c_str());
		pRoot->InsertEndChild(entityXML);


		map<string, string> varList = varListList.at(counter);
		for (auto& x : varList) {


			XMLElement * varXML = xmlDoc.NewElement("Variable");
			entityXML->InsertEndChild(varXML);

			XMLElement * varNameXML = xmlDoc.NewElement("Field");
			varNameXML->SetAttribute("Type", "VariableName");
			varNameXML->SetText((x.first).c_str());

			XMLElement * varTypeXML = xmlDoc.NewElement("Field");
			varTypeXML->SetAttribute("Type", "VariableType");
			varTypeXML->SetText((x.second).c_str());

			varXML->InsertFirstChild(varNameXML);
			varXML->InsertEndChild(varTypeXML);


		}

		counter++;
	}
	XMLError eResult = xmlDoc.SaveFile("OutputFile.xml");
	XMLCheckResult(eResult);

}

map<string, string> ReadXML(wchar_t* singleFileName = L"")
{
	//
	wchar_t folderPath[100] = { L"S:/Presagis/Suite16/STAGE/data/sm_db/default/me_mission/" };
	
	map<string, string> variableMap;
	vector<wchar_t*> files;


	//vector<wchar_t*>::iterator it;

	//if (singleFileName == L"")
	//{

	//	DIR *dir;
	//	struct dirent *ent;
	//	if ((dir = opendir(folderPath.c_str())) != NULL)
	//	{
	//		/* print all the files and directories within directory */
	//		while ((ent = readdir(dir)) != NULL)
	//		{

	//			//printf ("%s\n", ent->d_name);

	//			string data = ent->d_name;
	//			if (!strcmp(data.c_str(), ".") || !strcmp(data.c_str(), ".."))
	//			{
	//				// do nothing (straight logic)
	//			}
	//			else if (data.substr(data.find_last_of(".") + 1) == "me_mission")
	//			{

	//				files.push_back(data);
	//			}
	//		}
	//		closedir(dir);
	//	}
	//	else
	//	{
	//		/* could not open directory */
	//		perror("");

	//	}

	//}
	//else
	//{
		files.push_back(singleFileName);
	//}

	for (auto it = files.begin(); it != files.end(); ++it)
	{
		wchar_t missionName[40];
		wcsncpy(missionName, *it, 40);

		if (DEBUG)
		{

			sig_wprint(missionName);
			sig_print(" :in ReadXML \n");
		}

		
		wchar_t* filename = wcscat(folderPath, missionName);//"KillEnemy.me_mission";

		if (DEBUG)
		{

			sig_wprint(filename);
			sig_print(" : filename \n");
		}
		string outfile = "tempFile.xml";
		ifstream ifs(filename, std::ios::in | std::ios::binary | std::ios::ate);
		
		fstream myFile(outfile, std::ios::out | std::ios::trunc);
		ifstream::pos_type fsize = ifs.tellg();
		ifs.seekg(0, ios::beg);
		vector<char> bytes(fsize);



		ifs.read(&bytes[0], fsize);

		// Create string from vector
		string xml_str(&bytes[0], fsize);

		// Skip unsupported statements
		size_t pos = 0;
		while (true)
		{
			pos = xml_str.find_first_of("<", pos);
			if (xml_str[pos + 1] == '?' || // <?xml...
				xml_str[pos + 1] == '!')   // <!DOCTYPE... or [<!ENTITY...
			{
				// Skip this line
				pos = xml_str.find_first_of("\n", pos);
			}
			else
				break;
		}
		xml_str = xml_str.substr(pos);

		myFile << xml_str;
		ifs.close();
		myFile.close();
		//----------------------------------------//

		int errorCode = 0;
		tinyxml2::XMLDocument xmlDoc;
		XMLError eResult = xmlDoc.LoadFile("S:\\Presagis\\Suite16\\STAGE\\GUIPlugin\\tempFile.xml");
		XMLCheckResult(eResult);
		XMLNode * pRoot = xmlDoc.FirstChild();
		if (pRoot == nullptr)
			sig_print( "Error reading first child \n");


		//Ensures it loops every task group
		XMLElement * pElement = pRoot->FirstChildElement("Field")->FirstChildElement("DBList")->FirstChildElement("DBList_Instance");
		if (pElement == nullptr)
			sig_print("Error reading field \n");

		while (pElement != nullptr)
		{
			//For every databaseListInstance this is being performed
			//Ensures it loops every task group

			XMLElement * qElement = pElement->FirstChildElement("Field");
			bool found = false;
			while (qElement != nullptr)
			{
				//Get to datablock reference
				string type_name;
				const char * szAttributeText = nullptr;
				szAttributeText = qElement->Attribute("Type_Name");
				type_name = szAttributeText;
				if (type_name == "Datablock Reference")
				{
					found = true;
					break;
				}
				qElement = qElement->NextSiblingElement("Field");
			}

			
			if (found)
			{
				if (DEBUG)
				{
					sig_print("Root field found \n");
				}
				//qElement is Field->Dblist->DblistInstance->Field(DBLIST or DBREFERENCE)
				//string typeOfMission = qElement->GetText(); qElement->FirstChildElement("Name")->GetText();




				//Get to datablock list after field
				XMLElement * vElement = qElement->FirstChildElement("Datablock")->FirstChildElement("Field");
				bool found2 = false;
				while (vElement != nullptr)
				{
					string type_name;
					const char * szAttributeText = nullptr;
					szAttributeText = vElement->Attribute("Type_Name");
					type_name = szAttributeText;
					if (type_name == "Datablock List")
					{
						found2 = true;
						break;
					}

					vElement = vElement->NextSiblingElement("Field");
				}

				if (found2)
				{
					XMLElement * wElement = vElement->FirstChildElement("DBList")->FirstChildElement("DBList_Instance");

					while (wElement != nullptr)
					{
						//Iterates over every DBLIST I.E PROCEDURE

						XMLElement * xElement = wElement->FirstChildElement("Field");

						bool found3 = false;
						while (xElement != nullptr)
						{
							string type_name;
							const char * szAttributeText = nullptr;
							szAttributeText = xElement->Attribute("Type_Name");
							type_name = szAttributeText;
							if (type_name == "Datablock Choice")
							{
								found3 = true;
								break;
							}
							xElement = xElement->NextSiblingElement("Field");
						}

						if (found3)
						{
							//Find details of the procedure

							XMLElement * yElement = xElement->FirstChildElement("DBChoice")->FirstChildElement("Datablock");

							XMLElement * zElement = yElement->FirstChildElement("Field")->FirstChildElement("Value");
							const char* str = zElement->GetText();
							string textVal = str;

							if (textVal == "loc_var_create")
							{

								XMLElement * finalE = yElement->FirstChildElement("Field")->NextSiblingElement("Field")->NextSiblingElement("Field")->FirstChildElement("DBList")->FirstChildElement("DBList_Instance");

								XMLElement * varName = finalE->FirstChildElement("Field")->NextSiblingElement("Field")\
									->NextSiblingElement("Field")->FirstChildElement("Value");
								string nameOfVariable = varName->GetText();

								string varType = finalE->NextSiblingElement("DBList_Instance")->FirstChildElement("Field")\
									->NextSiblingElement("Field")->NextSiblingElement("Field")\
									->FirstChildElement("Value")->GetText();

								variableMap[nameOfVariable] = varType;
								//cout << nameOfVariable << " " << varType << endl;
							}
						}
						wElement = wElement->NextSiblingElement("DBList_Instance");
					}

				}

				if (DEBUG)
				{
					sig_print("Checking conditions event \n");
				}
				qElement = qElement->NextSiblingElement("Field");
				//qElement is now Db list - chcek for conditionions event
				XMLNode * nextNode = qElement->FirstChildElement("DBList")->FirstChild();
				if (nextNode != nullptr)
				{



					XMLElement * nextElement = nextNode->FirstChildElement("Field")->NextSiblingElement("Field")->NextSiblingElement("Field")->NextSiblingElement("Field")->FirstChildElement("DBList");
					if (nextElement != nullptr)
					{
						//Procedures iteratre through all creation of procedures
						XMLElement * procedures = nextElement->FirstChildElement("DBList_Instance");
						while (procedures != nullptr)
						{
							//varCreate is till datablock
							XMLElement * varCreateProc = procedures->FirstChildElement("Field")->\
								NextSiblingElement("Field")->FirstChildElement("DBChoice")->\
								FirstChildElement("Datablock");

							XMLElement * temp = varCreateProc->FirstChildElement("Field")->\
								FirstChildElement("Value");



							string saveIt = temp->GetText();
							if ((varCreateProc != nullptr) && (temp != nullptr) && (saveIt == "loc_var_create"))
							{
								//varControl is till database list instance
								XMLElement * varControl = varCreateProc->FirstChildElement("Field")->NextSiblingElement("Field")\
									->NextSiblingElement("Field")->FirstChildElement("DBList")->FirstChildElement("DBList_Instance");

								XMLElement * varNameXML = varControl\
									->FirstChildElement("Field")->NextSiblingElement("Field")->NextSiblingElement("Field")\
									->FirstChildElement("Value");

								if ((varControl != nullptr) && (varNameXML != nullptr))
								{
									const char* varNameChar;
									varNameChar = varNameXML->GetText();
									string varName = varNameChar;


									string varType = varControl->NextSiblingElement("DBList_Instance")->FirstChildElement("Field")\
										->NextSiblingElement("Field")->NextSiblingElement("Field")\
										->FirstChildElement("Value")->GetText();


									variableMap[varName] = varType;
									//cout << varName << " " << varType << endl;

								}
							}

							procedures = procedures->NextSiblingElement("DBList_Instance");
						}
					}
				}
				pElement = pElement->NextSiblingElement("DBList_Instance");
			}
		}

		if (DEBUG)
		{
			sig_print("File closed \n");
		}
		remove("tempFile.xml");
	}
	return variableMap;
}
/*-------------------------------------------------------------*/
/* find_config_arg()                                           */
/*                                                             */
/* Validates a command keyword                                 */
/*                                                             */
/* Return value: index of the keyword in the table or -1       */
/*-------------------------------------------------------------*/
static int find_config_arg(char *cur_arg)
{
	int tab_size;
	int i;

	tab_size = sizeof(tab_config_args) / sizeof(char *);

	/* Search the keyword in the internal table */
	for (i = 0; i < tab_size; i++)
	{
		if (!strcmp(cur_arg, tab_config_args[i]))
		{
			return(i);
		}
	}

	/* Keyword not found */
	return(-1);
}


/*-------------------------------------------------------------*/
/* store_stop_entity_name()                                    */
/*                                                             */
/* Stores the entity name whose death causes the run to end    */
/*                                                             */
/* Return value: none                                          */
/*-------------------------------------------------------------*/
static void store_stop_entity_name(char* buf_ptr)
{
	int i = 0;
	for (;;)
	{
		stop_entity_name[i] = (wchar_t)buf_ptr[i];
		if (buf_ptr[i] == '\0') break;
		++i;
	}
}

static void store_mission_entity_name(char* buf_ptr)
{
	int i = 0;
	for (;;)
	{
		mission_entity_name[i] = (wchar_t)buf_ptr[i];
		if (buf_ptr[i] == '\0') break;
		++i;
	}
}

/*-------------------------------------------------------------*/
/* parse_config_file()                                         */
/*                                                             */
/* Parses a batch file for SIM commands                        */
/*                                                             */
/* Return value: none                                          */
/*-------------------------------------------------------------*/
static void parse_config_file(void)
{
	char buffer[BUF_SIZE + 1], buf_copy[BUF_SIZE + 1], *buf_ptr, *buf_ptr2;
	FILE *file_ptr;
	int error = 0;
	int cur_line = 0;
	Entity_Data *ent;

	/* Open batch file */
	char file_name[256];
	sprintf(file_name, "%s%cGUIPlugin%c%s",
		stage_get_ansi_path(STAGE_DIR),
		SIG_PATH_SEP, SIG_PATH_SEP, SIM_CMD_FILE);
	file_ptr = fopen(file_name, "r");
	if (file_ptr == NULL)
	{
		sig_print("Error: Cannot open file %s for parsing !\n",
			SIM_CMD_FILE);
		sim_rtc_exit();
	}
	else
	{
		sig_print("Parsing SIM configuration file...\n");
	}

	for (;;)
	{

		/* Read a line from the batch file */
		fgets(buffer, BUF_SIZE, file_ptr);

		if (feof(file_ptr))
			break;

		cur_line++;

		/* Keep a copy of the buffer as strtok
		will write into the original... */
		strcpy(buf_copy, buffer);

		/* Retrieve first argument (command keyword) */
		buf_ptr2 = strtok(buffer, " \t\n");
		if (buf_ptr2 == NULL)
		{
			error = 1;
			break;
		}

		/* Retrieve second argument (value for the command) */
		buf_ptr = strtok(NULL, " \t\n");
		if (buf_ptr == NULL)
		{
			error = 1;
			break;
		}

		/* Validate the command keyword and execute
		the corresponding action, if any */
		switch (find_config_arg(buf_ptr2))
		{

		case 0:  /* Execute command */
		{
			ent = pi_get_entity(prd, 0);
			if (ent != NULL)
			{
				float speed = (float)atof(buf_ptr);

				sig_print("Ptf 0: setting requested speed to %f\n",
					speed);
				sim_ptf_change_speed(ent, speed);
			}
		}
		break;

		case 1:  /* Execute command */
		{
			ent = pi_get_entity(prd, 1);
			if (ent != NULL)
			{
				float altitude = (float)atof(buf_ptr);

				sig_print("Ptf 1: setting requested altitude to %f\n",
					altitude);
				sim_ptf_change_altitude(ent, altitude);
			}
		}
		break;

		case 2:  /* Retrieve value */
		{
			size_t bufflen = strlen(buf_ptr);
			wchar_t * buffUni = (wchar_t *)malloc(sizeof(*buffUni)*bufflen);
			swprintf(buffUni, bufflen, L"%ls", buf_ptr);
			ent = pi_get_entity(prd, 0);
			if (ent != NULL)
			{
				sig_print("Nr of weapons '%s' of entity 0 is %d\n",
					buf_ptr, sim_get_nr_wpn(ent, buffUni));
			}
		}
		break;

		/* Retrieve value */
		case 3:
		{
			size_t bufflen = strlen(buf_ptr);
			wchar_t * buffUni = (wchar_t *)malloc(sizeof(*buffUni)*bufflen);
			swprintf(buffUni, bufflen, L"%ls", buf_ptr);
			ent = pi_get_entity(prd, 1);
			if (ent != NULL)
			{
				sig_print("Nr of weapons '%s' of entity 1 is %d\n",
					buf_ptr, sim_get_nr_wpn(ent, buffUni));
			}
		}
		break;

		/* Random SEED */
		case 4:
		{
			seed = atol(buf_ptr);
		}
		break;

		/* RUN number */
		case 5:
		{
			run_number = atoi(buf_ptr);
		}
		break;

		/* ITERATION number */
		case 6:
		{
			iteration_number = atoi(buf_ptr);
		}
		break;

		/* STOP_CONDITION_TIME_ELAPSED */
		case 7:
		{
			stop_time = atof(buf_ptr);
		}
		break;

		/* STOP_CONDITION_ENTITY_DEAD */
		case 8:
		{
			store_stop_entity_name(buf_ptr);
		}
		break;

		/* STOP_CONDITION_MIN_RED_PLATFORM_COUNT */
		case 9:
		{
			min_red_platform_count = atoi(buf_ptr);
		}
		break;

		/* STOP_CONDITION_MIN_BLUE_PLATFORM_COUNT */
		case 10:
		{
			min_blue_platform_count = atoi(buf_ptr);
		}
		break;


		case -1: /* Error, command could not be validated */
			sig_print("Command keyword \"%s\" not recognized !\n", buf_ptr2);
			break;

		default: /* Warning, nothing to execute */
			sig_print("Nothing to execute for %s = %s !\n",
				buf_ptr2, buf_ptr);
			break;
		}
	}

	fclose(file_ptr);

	if (!cur_line)  /* Nothing to execute */
	{
		sig_print("Nothing to do for this run !\n");
	}

	if (error)
	{
		sig_print("Error: Parse error in file %s at line %i\n(%s)\n",
			SIM_CMD_FILE, cur_line, buf_copy);
		sim_rtc_exit();
	}
}

/***************************************************************/
/***************************************************************/
/* P U B L I C   F U N C T I O N S   C O D E   S E C T I O N   */
/***************************************************************/
/***************************************************************/
string WcharToString(wchar_t* s)
{
	//using namespace std;

	//wstring ws(wchr);
	//// your new String
	//string strMake(ws.begin(), ws.end());
	//
	//return strMake;

	char dfault = '?';
	std::ostringstream stm;
	const std::locale& loc = std::locale();
	while (*s != L'\0') {
		stm << std::use_facet< std::ctype<wchar_t> >(loc).narrow(*s++, dfault);
	}
	return stm.str();
	
	//std::string test((LPCTSTR)CString(s));
	//return std::wstring(s);

}

void StringToWchar(std::string str, wchar_t*& wchr)
{
	using namespace std;
	/*This Block converts char to wchar pointer */
	//char* strVar = "MyOwnVariable";
	wchr = new wchar_t[str.size()];
	int i = 0;
	for (;;)
	{
		wchr[i] = (wchar_t)str[i];
		if (str[i] == '\0') break;
		++i;
	}
}


void HandleData(vector<string> & entityList, vector<wstring> & mlist)
{
	//This function takes in vector list of entities and their associated mission, and extracts their variables
	//extracted variables along with entity names are then written to a file
	
	

	vector<string> ent(entityList);
	
	vector<map<string, string>> varList;
	

	for (auto i = mlist.begin(); i != mlist.end(); ++i)
	{
		wchar_t missionName[40]; 
		wcsncpy(missionName, (*i).c_str() , 40);
		map<string, string> reader = ReadXML(missionName);
		varList.push_back(reader);
	}
	
	WriteXML(entityList, varList);
}

void SaveDataForPublish()
{
	Entity_Data* entity;
	int i;
	for (i = 0; i < gdp->priv->config.max_nr_entities; i++)
	{
		entity = &gdp->priv->ent[i];
		if (entity_is_ptf(entity))
		{


			sig_wprint(entity->state->name);

		}
	}
}

/*-------------------------------------------------------------*/
/* um_batch_init()                                             */
/*                                                             */
/* Saves the pointers to the global and private databases      */
/*                                                             */
/* Return value: 0 on success; -1 on error                     */
/*-------------------------------------------------------------*/
int um_batch_init(void *gd)
{
	gdp = (Sim_Global_Data *)gd;
	prd = gdp->priv;


	rbidx_ptr = &gdp->pub->header->rbuff_idx;

	sig_print("Stage Batch Sim Plugin Initialized\n");
	// SaveDataForPublish();
	init = true;
	//CreateXML();
	//ReadXML();
	return(0);
}

/*-------------------------------------------------------------*/
/* um_batch_ctrl()                                             */
/*                                                             */
/* Function called when RTS events occur                       */
/*                                                             */
/* Return value: none                                          */
/*-------------------------------------------------------------*/
void um_batch_ctrl(int ctrl_type)
{
	switch (ctrl_type)
	{
	case CTRL_START:
	{
		char out_file_name[SIG_PATH_MAX];
		time_t rawtime;
		struct tm timeinfo;
		/* Parse file when SIM is given Run command */
		parse_config_file();

		rawtime = time(NULL);
		timeinfo = *localtime(&rawtime);

		sprintf(out_file_name, "out_data.%d.%d", run_number, iteration_number);
		out_file = fopen(out_file_name, "w");

		if (out_file == NULL)
		{
			sig_print("um_batch_ctrl:Error opening out file <%s>\n", out_file_name);
		}

		/* Display start message and stop conditions */
		sig_print("Starting run #%d, iteration %d with seed %d\n",
			run_number, iteration_number, seed);
		sig_print("Stop conditions: ");
		if (stop_time > 0)
		{
			sig_print("after %f seconds, ", (float)stop_time);
		}

		if (stop_entity_name[0] != L'\0')
		{
			sig_wprint(L"when '%ls' dies, ", &stop_entity_name[0]);
		}

		if (min_red_platform_count > 0 || min_blue_platform_count > 0)
		{
			sig_print("when under %d red or %d blue platforms, ",
				min_red_platform_count, min_blue_platform_count);
		}

		sig_print("\n");

		stopped = FALSE;
	}
	break;

	case CTRL_RESET:
	{
		if (seed != 0)
		{
			sig_srand48(seed);/* Set random generator seed if selected in command file */
		}
	}
	break;

	case CTRL_STOP:
	{
		stop_time = -1;
		stop_entity_name[0] = L'\0';
		min_red_platform_count = 0;
		min_blue_platform_count = 0;
		if (out_file)
		{
			fclose(out_file);
			out_file = NULL;
		}
	}
	break;

	default:
	{
	}
	}
}

size_t split(const std::string &txt, std::vector<std::string> &strs, char ch)
{
	size_t pos = txt.find(ch);
	size_t initialPos = 0;
	strs.clear();

	// Decompose statement
	while (pos != std::string::npos) {
		strs.push_back(txt.substr(initialPos, pos - initialPos));
		initialPos = pos + 1;

		pos = txt.find(ch, initialPos);
	}

	// Add the last one
	strs.push_back(txt.substr(initialPos, min(pos, txt.size()) - initialPos + 1));

	return strs.size();
}

int CheckActivatedMission(int debug)
{

	int ret_val = 0;
	fstream missionFile(SIM_MISSION_FILE, ios::in);
	string line;
	
	if (missionFile.is_open())
	{
		
			while (getline(missionFile, line))
			{

				string newCom = "NEW_COMMAND";
				
				if ((line.find(newCom)) != 0)
				{

					sig_print("No new commands found \n");
					continue;
				}
				std::vector<std::string> v;
				split(line, v, ' ');



			
				Commands newCommands;
				newCommands.entityName = v.at(1);
				newCommands.variableName = v.at(2);
				newCommands.value = v.at(3);
				CommandQueue.push_back(newCommands);

				ret_val = 1;

			}
			
		
			missionFile.close();

			if (ret_val == 1)
			{
				missionFile.open(SIM_MISSION_FILE, std::ofstream::out | std::ofstream::trunc);
				missionFile.close();
			}
		
	}

	return ret_val;
	
}
/*-------------------------------------------------------------*/
/* um_batch()                                                  */
/*                                                             */
/* Function called every iteration to verify if max time for   */
/* run has been reached.                                       */
/*                                                             */
/* Return value: none                                          */
/*-------------------------------------------------------------*/
void um_batch_exec()
{
	
	if (checkonthis % 500 == 0)
	{
		checkonthis = 1;
		if (CheckActivatedMission(1) == 1)
		{			
			sig_print("New mission commands found \n");
		}
		else
		{
			sig_print("Nothing to execute \n");
		}

	}
	else
	{
		checkonthis = checkonthis + 1;
	}

	int i;
	Entity_Data* entity;
	int stop_entity_is_alive = 0;
	int alive_red_platform_count = 0;
	int alive_blue_platform_count = 0;
	int should_stop = 0;

	/* Check if the stop simulation time is reached */
	if (!stopped
		&& stop_time > 0.0
		&& prd->system_data.sim_time >= stop_time)
	{
		sig_print("Stop condition reached: Sim time %f reached.\n",
			(float)stop_time);
		should_stop = 1;
	}

	if (gdp == NULL)
		sig_print("Warning - Global Simulation data is empty \n");
	/* Check if the entity whose death causes the run to stop is alive */

	std::vector<std::string> entityList;
	std::vector<wstring> missionList;
	
	for (i = 0; i < gdp->priv->config.max_nr_entities; i++)
	{
		
		entity = &gdp->priv->ent[i];
		if (entity_is_allocated(entity)
			&& !entity_is_wreck(entity)
			&& entity_is_ptf(entity)
			&& entity->scn_ptf != NULL)
		{

			//WriteData to publish
			if (init)
			{
				
				Task_Ent_Data* op = entity->task;
				if (op != NULL)
				{
					if (op->mission != NULL)
					{
						
						wchar_t* temp1 = (entity->state->name);
						wchar_t temp2[40];
						wcsncpy(temp2, (op->mission_name), 40);
						if (temp2 != NULL && temp1!=NULL)
						{


							wchar_t* extension = L".me_mission";
							wchar_t* withExtension = wcscat(temp2, extension);
							

							string tempEntity = WcharToString(temp1);

							entityList.push_back(tempEntity);
							missionList.push_back(withExtension);
						}
					}
				}

				initialEntities++;
			}

			/*if (wcscmp(entity->scn_ptf->scn_ptf_name, stop_entity_name) == 0)
			{
			stop_entity_is_alive = 1;
			entity->scn_ptf->mission_active = GSIM_SCN_PTF_MISSION_ACTIVE_YES;


			}*/

			for (auto i = CommandQueue.begin(); i != CommandQueue.end(); ++i)
			{
				wchar_t * entityName;

				StringToWchar((*i).entityName, entityName);
				
				//wcsncpy(missionName, (*i).c_str(), 40);

				/*If entity name equals mission entitiy name*/
				if (wcscmp(entity->scn_ptf->scn_ptf_name, entityName) == 0)
				{
					
					string variableNameString = (*i).variableName;
						

					Task_Ent_Data* op = entity->task;

					if (op == NULL)
					{

						sig_print("No Mission found");
					}
					else
					{
						wchar_t* temp = op->mission_name;
						sig_print("Mission ");
						sig_wprint(temp);
						sig_print(" Found \n");

						/*DeComment this block to make mission active again*/
						// int missionStart = sim_task_start_mission(entity);
						// if (missionStart == 0)
						// {
						//	 sig_print("Success \n");
						// }

						/*This Block converts char to wchar pointer */
						//std::string strVar = "MyOwnVariable";
						wchar_t* variableName;
						StringToWchar(variableNameString, variableName);


						simValue* local = sim_task_get_local_variable(entity, variableName);
						
						if (local == NULL)
						{
							sig_print("Failed - Variable not found \n");

						}
						else
						{
							
							string varValue = (*i).value;
							
							if (UpdateVariableValue(local, varValue) == 0)
							{
								sig_print("Value of Variable changed \n");		
							}
							else
							{
								sig_print("Error changing value for variable: %s\n", variableNameString);
							}
							
						}

					}

						//entity->active_time = prd->system_data.sim_time;
						
				}


				

			}

			

			


			if (entity->scn_ptf->mission_active == GSIM_SCN_PTF_MISSION_ACTIVE_NO)
			{
				//	sig_print("Mission activated\n");
				//	entity->scn_ptf->mission_active = GSIM_SCN_PTF_MISSION_ACTIVE_YES;
			}

			switch (entity->scn_ptf->color)
			{
			case GSIM_SCN_PTF_COLOR_PTF_ID_BLUE:
				alive_blue_platform_count++;
				break;

			case GSIM_SCN_PTF_COLOR_PTF_ID_RED:
				alive_red_platform_count++;
				break;
			}
		}
	}

	CommandQueue.clear();

	if (init == true )
	{
		HandleData(entityList, missionList);
		DWORD ecode;
		BOOL succ = PublishData(ecode);
		if (ecode == 0)
		{
			sig_print("Data transmission successful \n");
		}
		else
		{
			if (ecode == 259)
			{
				sig_print("Subscriber not found. \n");
			}
			sig_print("Publisher failed! Ending RTC to avoid standalone play \n");
			should_stop = 1;
		}
	}
	init = false;

	/*if (alive_red_platform_count < min_red_platform_count
		|| alive_blue_platform_count < min_blue_platform_count)
	{
		sig_print("Stop condition reached: %d/%d red and %d/%d blue platforms remain.\n",
			alive_red_platform_count, min_red_platform_count,
			alive_blue_platform_count, min_blue_platform_count);
		should_stop = 1;
	}

	if (stop_entity_name[0] != L'\0' && !stop_entity_is_alive)
	{
		sig_wprint(L"Stop condition reached: Entity '%ls' is dead.\n",
			&stop_entity_name[0]);
		should_stop = 1;
	}*/

	if (should_stop)
	{
		stopped = TRUE;
		init = true;
		sim_rtc_stop(0);
	}
	else
	{
		um_batch_grab_data();
	}
}
void KillProcess()
{
	HANDLE hProcessSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);

	PROCESSENTRY32 ProcessEntry = { 0 };
	ProcessEntry.dwSize = sizeof(ProcessEntry);

	BOOL Return = FALSE;
Label:Return = Process32First(hProcessSnapShot, &ProcessEntry);

	if (!Return)
	{
		goto Label;
	}

	do
	{
		int value = _tcsicmp(ProcessEntry.szExeFile, _T("publisher.exe"));
		//replace the taskmgr.exe to the process u want to remove.
		if (value == 0)
		{
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, ProcessEntry.th32ProcessID);
			TerminateProcess(hProcess, 0);
			CloseHandle(hProcess);
		}

	} while (Process32Next(hProcessSnapShot, &ProcessEntry));

	CloseHandle(hProcessSnapShot);
}
BOOL PublishData(DWORD & exitCode)
{
	
	KillProcess();
	
	CString cmdLine = "\"S:\\Presagis\\Suite16\\STAGE\\GUIPlugin\\subscriber\\publisher.exe\" -DCPSConfigFile \"S:\\Presagis\\Suite16\\STAGE\\GUIPlugin\\subscriber\\rtps.ini\"";
	PROCESS_INFORMATION processInformation = { 0 };
	STARTUPINFO startupInfo = { 0 };
	startupInfo.cb = sizeof(startupInfo);
	int nStrBuffer = cmdLine.GetLength() + 50;


	// Create the process

	BOOL result = CreateProcess(NULL, cmdLine.GetBuffer(nStrBuffer),
		NULL, NULL, FALSE,
		NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,
		NULL, NULL, &startupInfo, &processInformation);
	cmdLine.ReleaseBuffer();


	if (!result)
	{
		// CreateProcess() failed
		// Get the error from the system


		// We failed.
		return FALSE;
	}
	else
	{
		sig_print("Publisher started, Waiting for subscriber acknowledgemnt (timeout in 30 seconds) \n");
		// Successfully created the process.  Wait for it to finish.
		WaitForSingleObject(processInformation.hProcess, 30000);

		// Get the exit code.
		result = GetExitCodeProcess(processInformation.hProcess, &exitCode);
		sig_print("status code %d \n", exitCode);
		
		// Close the handles.
		CloseHandle(processInformation.hProcess);
		CloseHandle(processInformation.hThread);

		if (!result)
		{
			// Could not get exit code.
			sig_print("status code %d \n", exitCode);
			if (exitCode == 259)
				KillProcess();
			return FALSE;
		}


		// We succeeded.
		return TRUE;
	}
}

int UpdateVariableValue(simValue*& local, string varValue)
{
	sim_Value_Types varType = local->GetType();
	int errorCode = 1;


	if (varType == SIM_BOOL)
	{

		transform(varValue.begin(), varValue.end(), varValue.begin(), ::tolower);

		if (varValue == "true")
			varValue = "1";
		else if (varValue == "false")
			varValue = "0";
		bool updateValueVar = ConvertString<bool>(varValue, errorCode);

		if (errorCode == 0)
		{
			local->operator=(updateValueVar);
		}

	}

	if (varType == SIM_INT)
	{
		int updateValueVar = ConvertString<int>(varValue, errorCode);
		if (errorCode == 0)
		{
			local->operator=(updateValueVar);
		}

	}

	if (varType == SIM_DOUBLE)
	{
		double updateValueVar = ConvertString<double>(varValue, errorCode);
		if (errorCode == 0)
		{
			local->operator=(updateValueVar);
		}

	}

	if (varType == SIM_FLOAT)
	{
		float updateValueVar = ConvertString<float>(varValue, errorCode);
		if (errorCode == 0)
		{

			local->operator=(updateValueVar);
		}

	}

	if (varType == SIM_STRING)
	{
		local->operator=(varValue);
	}

	return errorCode;
	

}

/*-------------------------------------------------------------*/
/* um_batch_grab_data()                                        */
/*                                                             */
/* Function called every iteration to save data that would     */
/* be interesting for later analysis.                          */
/*                                                             */
/* Return value: none                                          */
/*-------------------------------------------------------------*/
void um_batch_grab_data(void)
{
	int i;

	if (out_file)
	{
		Entity_Data *ent;
		for (ent = prd->ent, i = 0; i < prd->config.max_nr_entities; i++, ent++)
		{
			if (entity_is_active(ent))
			{
				Actual *act = NULL;
				float speed = 0;
				float heading = 0;
				float alt = 0;
				float h_ground = 0;
				float dam = 0;
				if (ent->dyn)
				{
					act = &(ent->dyn->perm_data.act);
					speed = act->speed;
					heading = act->grnd_track;
					alt = act->altitude;
					h_ground = act->ground_elevation;
					if (ent->scoring) dam = ent->scoring->perm_data.damages;
				}
				/*
				* This form can be used to import the data
				* inside a spreadsheet using spaces as separators
				*/
				fprintf(out_file, "%f %ls %f %f %f %f %f\n",
					prd->system_data.sim_time, /* Save simulation time */
					ent->state->name,          /* Save name of entity */
					dam,                       /* Save damage level 0-100 */
					speed,                     /* Save speed */
					alt,                       /* Save altitude */
					h_ground,                  /* Save ground elevation */
					heading);                  /* Save heading */
			}
		}
	}
}

void um_tmpl_daemon()
{
}


/*-----------------------------------------------------------------------*/
/* um_tmpl()    This routine is executed every iteration                 */
/*                                                                       */
/* returned value: none                                                  */
/*-----------------------------------------------------------------------*/
void um_tmpl_background()
{

}
