/*
 *
 *
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

#include <ace/Log_Msg.h>
#include <ace/OS_NS_stdlib.h>

#include "DataReaderListenerImpl.h"
#include "DatatransferTypeSupportC.h"
#include "DatatransferTypeSupportImpl.h"

#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
using namespace std;
void ReadCSV(string vars, string vartype);
void
DataReaderListenerImpl::on_requested_deadline_missed(
  DDS::DataReader_ptr /*reader*/,
  const DDS::RequestedDeadlineMissedStatus& /*status*/)
{
}

void
DataReaderListenerImpl::on_requested_incompatible_qos(
  DDS::DataReader_ptr /*reader*/,
  const DDS::RequestedIncompatibleQosStatus& /*status*/)
{
}

void
DataReaderListenerImpl::on_sample_rejected(
  DDS::DataReader_ptr /*reader*/,
  const DDS::SampleRejectedStatus& /*status*/)
{
}

void
DataReaderListenerImpl::on_liveliness_changed(
  DDS::DataReader_ptr /*reader*/,
  const DDS::LivelinessChangedStatus& /*status*/)
{
}

void
DataReaderListenerImpl::on_data_available(DDS::DataReader_ptr reader)
{
  Datatransfer::VariablesDataReader_var reader_i =
    Datatransfer::VariablesDataReader::_narrow(reader);

  if (!reader_i) {
    ACE_ERROR((LM_ERROR,
               ACE_TEXT("ERROR: %N:%l: on_data_available() -")
               ACE_TEXT(" _narrow failed!\n")));
    ACE_OS::exit(-1);
  }

  Datatransfer::Variables message;
  DDS::SampleInfo info;

  DDS::ReturnCode_t error = reader_i->take_next_sample(message, info);

  if (error == DDS::RETCODE_OK) {
    std::cout << "SampleInfo.sample_rank = " << info.sample_rank << std::endl;
    std::cout << "SampleInfo.instance_state = " << info.instance_state << std::endl;

	

    if (info.valid_data) {
		std::cout << "         id = " << message.var_id << std::endl
			<< "         entity= " << message.entity.in() << std::endl
			<< "         count= " << message.count << std::endl; 

		cout << "Reading Variable and their types" << endl;
		ReadCSV(message.vars.in(), message.type.in());

			



    }

  } else {
    ACE_ERROR((LM_ERROR,
               ACE_TEXT("ERROR: %N:%l: on_data_available() -")
               ACE_TEXT(" take_next_sample failed!\n")));
  }
}

void
DataReaderListenerImpl::on_subscription_matched(
  DDS::DataReader_ptr /*reader*/,
  const DDS::SubscriptionMatchedStatus& /*status*/)
{
}

void
DataReaderListenerImpl::on_sample_lost(
  DDS::DataReader_ptr /*reader*/,
  const DDS::SampleLostStatus& /*status*/)
{
}

void ReadCSV(string vars,string vartype)
{
	



	bool condition = true;
	string str = "";
	string vartypeSave = "";
	int pos = 0;
	int posT = 0;
	int OFSET = 0;
	int commaCount = 0;
	int accStrLength = 0;
	int accTypeLength = 0;
	do
	{
		accStrLength = accStrLength + str.length();
		pos = vars.find(",", pos + OFSET);

		accTypeLength = accTypeLength + vartypeSave.length();
		posT = vartype.find(",", posT + OFSET);

		if (pos>1)
		{
			str = vars.substr(accStrLength + commaCount, pos - accStrLength - commaCount);
			vartypeSave = vartype.substr(accTypeLength + commaCount, posT - accTypeLength - commaCount);
		}
		else
		{
			str = vars.substr(accStrLength + commaCount, pos);
			vartypeSave = vartype.substr(accTypeLength + commaCount, posT);
		}

		cout << str << " " << vartypeSave << endl;


		commaCount++;
		OFSET = 1;
		condition = !(pos == string::npos);
	} while (condition);
}
