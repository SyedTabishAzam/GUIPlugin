/*
 *
 *
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <ace/Log_Msg.h>
#include <tuple>
#include <dds/DdsDcpsInfrastructureC.h>
#include <dds/DdsDcpsPublicationC.h>
#include <sys/stat.h>
#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/WaitSet.h>

#include "dds/DCPS/StaticIncludes.h"

#include "DatatransferTypeSupportImpl.h"
#include "tinyxml2.h"
#include "tinyxml2.cpp"
using namespace tinyxml2;
using namespace std;

#ifndef XMLCheckResult
#define XMLCheckResult(a_eResult) if (a_eResult != XML_SUCCESS) { printf("Error: %i\n", a_eResult); }
#endif

vector<tuple<string, string, string,int >> ParseVariableFile();
const int NUM_SECONDS = 3;

void SendStatusToSubscribers(Datatransfer::VariablesDataWriter_var&,int);
void SendServerData(Datatransfer::VariablesDataWriter_var&);
const string XML_OUTPUT_FILE = "OutputFile.xml";
const string SERVER_DATA = "Server.dat";

int count = 1;
double time_counter = 0;
clock_t this_time = clock();
clock_t last_time = this_time;

enum CONNECTION_CODE { ACK = 500, SERVER_UPDATE = 501, SCENERIO_UPDATE = 502, EXIT = 503, ERROR_REC = 504 };
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
	strs.push_back(txt.substr(initialPos, std::min(pos, txt.size()) - initialPos + 1));

	return strs.size();
}

int ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{

  try {
	  int fps = 0;
    // Initialize DomainParticipantFactory
    DDS::DomainParticipantFactory_var dpf = TheParticipantFactoryWithArgs(argc, argv);

    // Create DomainParticipant
    DDS::DomainParticipant_var participant = dpf->create_participant(43,PARTICIPANT_QOS_DEFAULT,0,OpenDDS::DCPS::DEFAULT_STATUS_MASK);

    if (!participant)
    {
      ACE_ERROR_RETURN((LM_ERROR,
                        ACE_TEXT("ERROR: %N:%l: main() -")
                        ACE_TEXT(" create_participant failed!\n")),
                       -1);
    }

    // Register TypeSupport (Messenger::Message)
    Datatransfer::VariablesTypeSupport_var ts = new Datatransfer::VariablesTypeSupportImpl;

    if (ts->register_type(participant, "") != DDS::RETCODE_OK) {
      ACE_ERROR_RETURN((LM_ERROR,
                        ACE_TEXT("ERROR: %N:%l: main() -")
                        ACE_TEXT(" register_type failed!\n")),
                       -1);
    }

    // Create Topic
    CORBA::String_var type_name = ts->get_type_name();
    DDS::Topic_var topic =
      participant->create_topic("Variable list",
                                type_name,
                                TOPIC_QOS_DEFAULT,
                                0,
                                OpenDDS::DCPS::DEFAULT_STATUS_MASK);

    if (!topic) {
      ACE_ERROR_RETURN((LM_ERROR,
                        ACE_TEXT("ERROR: %N:%l: main() -")
                        ACE_TEXT(" create_topic failed!\n")),
                       -1);
    }

    // Create Publisher
    DDS::Publisher_var publisher =
      participant->create_publisher(PUBLISHER_QOS_DEFAULT,
                                    0,
                                    OpenDDS::DCPS::DEFAULT_STATUS_MASK);

    if (!publisher) {
      ACE_ERROR_RETURN((LM_ERROR,
                        ACE_TEXT("ERROR: %N:%l: main() -")
                        ACE_TEXT(" create_publisher failed!\n")),
                       -1);
    }

    // Create DataWriter
    DDS::DataWriter_var writer =
      publisher->create_datawriter(topic,
                                   DATAWRITER_QOS_DEFAULT,
                                   0,
                                   OpenDDS::DCPS::DEFAULT_STATUS_MASK);

    if (!writer) {
      ACE_ERROR_RETURN((LM_ERROR,
                        ACE_TEXT("ERROR: %N:%l: main() -")
                        ACE_TEXT(" create_datawriter failed!\n")),
                       -1);
    }

    Datatransfer::VariablesDataWriter_var message_writer = Datatransfer::VariablesDataWriter::_narrow(writer);

    if (!message_writer) {
      ACE_ERROR_RETURN((LM_ERROR,
                        ACE_TEXT("ERROR: %N:%l: main() -")
                        ACE_TEXT(" _narrow failed!\n")),
                       -1);
    }
	printf("PUBLISHER STARTED\n");
    // Block until Subscriber is available
    DDS::StatusCondition_var condition = writer->get_statuscondition();
    condition->set_enabled_statuses(DDS::PUBLICATION_MATCHED_STATUS);

    DDS::WaitSet_var ws = new DDS::WaitSet;
    ws->attach_condition(condition);
	int initialCount = 0;
    while (true) {
      DDS::PublicationMatchedStatus matches;
      if (writer->get_publication_matched_status(matches) != ::DDS::RETCODE_OK) {
        ACE_ERROR_RETURN((LM_ERROR,
                          ACE_TEXT("ERROR: %N:%l: main() -")
                          ACE_TEXT(" get_publication_matched_status failed!\n")),
                         -1);
      }

      if (matches.current_count >= 1) {

		  //On new subscriber join, send acknowledgement
		  if (initialCount != matches.current_count)
		  {
			  
			
			  initialCount = matches.current_count;
			  printf("Subscriber %d joined the server \n", initialCount);
			  SendStatusToSubscribers(message_writer, initialCount);
		  }

		  SendServerData(message_writer);
		  //Break the loop when execution starts
		  ifstream myFile(XML_OUTPUT_FILE);
		  if (!myFile.fail()){
			  myFile.close();
			  break;
		  }
		  
		 
		 
      }

    /*  DDS::ConditionSeq conditions;
      DDS::Duration_t timeout = { 300, 0 };
      if (ws->wait(conditions, timeout) != DDS::RETCODE_OK) {
        ACE_ERROR_RETURN((LM_ERROR,
                          ACE_TEXT("ERROR: %N:%l: main() -")
                          ACE_TEXT(" wait failed!\n")),
                         -1);
      }*/
    }

    //ws->detach_condition(condition);
	cout << "Server Started! Transfering server data on loop" << endl;
	
	while (true)
	{
		ifstream myFile(XML_OUTPUT_FILE);
		if (myFile.fail()){
			myFile.close();
			continue;
		}
		vector<tuple<string, string, string, int >> circular = ParseVariableFile();

		vector<tuple<string, string, string, int >>::iterator i;

		
		for (i = circular.begin(); i != circular.end(); i++)
		{
			Datatransfer::Variables variable;
		

			string entityName = get<0>(*i);
			string variableName = get<1>(*i);
			string variableType = get<2>(*i);
			int count = get<3>(*i);

			variable.var_id = SCENERIO_UPDATE;
			variable.entity = entityName.c_str();
			variable.count = count;
			variable.vars = variableName.c_str();
			variable.type = variableType.c_str();
		
	

	


			DDS::ReturnCode_t error = message_writer->write(variable, DDS::HANDLE_NIL);
			++variable.count;
			++variable.var_id;

			if (error != DDS::RETCODE_OK) {
				ACE_ERROR((LM_ERROR,
					ACE_TEXT("ERROR: %N:%l: main() -")
					ACE_TEXT(" write returned %d!\n"), error));
			}


					// Wait for samples to be acknowledged
			DDS::Duration_t timeout = { 30, 0 };
			if (message_writer->wait_for_acknowledgments(timeout) != DDS::RETCODE_OK) {
				ACE_ERROR_RETURN((LM_ERROR,
					ACE_TEXT("ERROR: %N:%l: main() -")
					ACE_TEXT(" wait_for_acknowledgments failed!\n")),
					-1);
           
			}
			

		}
		myFile.close();
		remove(XML_OUTPUT_FILE.c_str());
		
		
		
	}
    // Clean-up!
	
    participant->delete_contained_entities();
    dpf->delete_participant(participant);
	
	
    TheServiceParticipant->shutdown();

  } catch (const CORBA::Exception& e) {
    e._tao_print_exception("Exception caught in main():");
    return -1;
  }

  return 0;
}

vector<tuple<string, string, string,int >> ParseVariableFile()
{
	XMLDocument xmlDoc;
	string filepath = XML_OUTPUT_FILE;
	XMLError eResult = xmlDoc.LoadFile(filepath.c_str());
	XMLCheckResult(eResult);

	vector<tuple<string, string, string, int >> circular;
	//Root element is the Associated tag
	XMLNode * pRoot = xmlDoc.FirstChild();
	if (pRoot == nullptr) cout << XML_ERROR_FILE_READ_ERROR << endl;


	//Iterate over every found entity
	XMLNode * entities = pRoot->FirstChild();
	while (entities != nullptr)
	{

		string entityname;
		entityname = entities->ToElement()->Name();

		//For each entity find all its variables and iterate
		XMLElement * varNode = entities->FirstChildElement("Variable");


		//If no variable found then continue
		if (varNode == nullptr)
		{
			entities = entities->NextSibling();
			continue;
		}

		int varCount = 0;
		string variableNameString = "";
		string variableTypeString = "";
		while (varNode != nullptr)
		{
			// Extract variable name
			XMLElement * variableName = varNode->FirstChildElement("Field");


			//Extract variable type
			XMLElement * variableType = variableName->NextSiblingElement("Field");

			if (varCount == 0)
			{
				const char * textHolder = nullptr;
				textHolder = variableName->GetText();
				variableNameString = textHolder;



				textHolder = variableType->GetText();
				variableTypeString = textHolder;



			}
			else
			{
				const char * textHolder = nullptr;
				textHolder = variableName->GetText();
				string temporary = textHolder;
				variableNameString += "," + temporary;

				textHolder = variableType->GetText();
				string temporary2 = textHolder;
				variableTypeString += "," + temporary2;
			}

			//Iterate and go to next variable
			varCount++;
			varNode = varNode->NextSiblingElement("Variable");
		}
		//Record data in vector
		circular.push_back(make_tuple(entityname, variableNameString, variableTypeString, varCount));
		entities = entities->NextSibling();
	}
	
	return circular;
}

void SendStatusToSubscribers(Datatransfer::VariablesDataWriter_var& msgWriter,int subcount)
{
	Datatransfer::Variables variable;


	

	variable.var_id = ACK;
	variable.count = subcount;
	variable.entity = "SERVER";
	
	variable.vars = "";
	variable.type = "";






	DDS::ReturnCode_t error = msgWriter->write(variable, DDS::HANDLE_NIL);
}

void SendServerData(Datatransfer::VariablesDataWriter_var& msgWriter)
{
	
	this_time = clock();

	time_counter += (double)(this_time - last_time);

	last_time = this_time;

	//if N seconds has passed then read file. otherwise continue
	if (time_counter > (double)(NUM_SECONDS * CLOCKS_PER_SEC))
	{
		time_counter -= (double)(NUM_SECONDS * CLOCKS_PER_SEC);

		//Read a file
		string line;
		fstream server;

		server.open(SERVER_DATA, ios::in);


		

		bool dataRead = true;
		if (server.is_open())
		{
			//int LineCount = std::count(std::istreambuf_iterator<char>(server),std::istreambuf_iterator<char>(), '\n');

			int defCount = 0;
			int attCount = 0;
			int cccCount = 0;
			while (getline(server, line))
			{


				std::vector<std::string> v;
				split(line, v, ' ');
				if (v.at(0) == "SERVER_DATA")
				{
					string playerType = v.at(1);
					if (playerType == "CCC")
					{
						cccCount++;
					}
					else if (playerType == "DEFENDER")
					{
						defCount++;
					}
					else if (playerType == "ATTACKER")
					{
						attCount++;
					}
				}
			}

			Datatransfer::Variables variable;




			variable.var_id = SERVER_UPDATE;
			variable.count = defCount + attCount + cccCount;
			variable.entity = to_string(defCount).c_str();
			variable.vars = to_string(attCount).c_str();
			variable.type = to_string(cccCount).c_str();

			DDS::ReturnCode_t error = msgWriter->write(variable, DDS::HANDLE_NIL);
		}
	}
}