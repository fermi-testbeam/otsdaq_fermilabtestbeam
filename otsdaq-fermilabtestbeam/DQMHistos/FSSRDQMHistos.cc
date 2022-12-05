#include "otsdaq-fermilabtestbeam/DQMHistos/FSSRDQMHistos.h"
#include "otsdaq-fermilabtestbeam/DataDecoders/FSSRData.h"
//#include "otsdaq-fermilabtestbeam/DataDecoders/VIPICData.h"
#include "otsdaq/ConfigurationInterface/ConfigurationTree.h"
#include "otsdaq/NetworkUtilities/NetworkConverters.h"

#include <iostream>
#include <sstream>
#include <string>

#include <TCanvas.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TFrame.h>
#include <TH1.h>
#include <TH1I.h>
#include <TH2.h>
#include <TH2I.h>
#include <TProfile.h>
#include <TROOT.h>
#include <TRandom.h>
#include <TThread.h>

#include <stdint.h>
//#include <arpa/inet.h>

// ROOT documentation
// http://root.cern.ch/root/html/index.html

using namespace ots;

//========================================================================================================================
// FSSRDQMHistos::FSSRDQMHistos(std::string supervisorApplicationUID, std::string
// bufferUID, std::string processorUID) : theDataDecoder_    (supervisorApplicationUID,
// bufferUID, processorUID) , bufferUID_         (bufferUID) , processorUID_
//(processorUID)
//{
//}

//========================================================================================================================
FSSRDQMHistos::FSSRDQMHistos(void) {}

//========================================================================================================================
FSSRDQMHistos::~FSSRDQMHistos(void) {}

//========================================================================================================================
void FSSRDQMHistos::book(TDirectory*              myDirectory,
                         const ConfigurationTree& theXDAQContextConfigTree,
                         const std::string&       configurationPath)
{
	clear();
	// __COUT__ << "Booking start!" << __E__;
	// __COUT__ << theXDAQContextConfigTree.getNode(configurationPath) << __E__;
	// __COUT__ << configurationPath << __E__;
	// __COUT__ << theXDAQContextConfigTree.getBackNode(configurationPath, 7) << __E__;
	// for(auto& context : theXDAQContextConfigTree.getChildren())
	// 	for(auto& application :
	// 	    context.second.getNode("LinkToApplicationTable").getChildren())
	// 		for(auto& supervisor : application.second.getChildren())
	// 			__COUT__ << context.first << " -> " << application.first << " -> "
	// 			         << supervisor.first
	// 			         << " -> "
	// 			         //				<< plugin.first
	// 			         << __E__;
	// __COUT__ << "NOW SELECT MINE" << __E__;

	// The assumption is that the interface streams to a data manager and the data manager
	// streams to the visualizer  Interface -> DMProducer -> DMConsumer -> VisualProducer
	// StreamTo  -> HostListener -> StreamTo -> HostListener

	// Interface->      StreamToIP   StreamToPort                  UID          Channels
	std::map<std::pair<std::string, unsigned int>,
	         std::multimap<std::string, unsigned int>>
	    interfaceStreamToMap;

	// FOR EACH PRODUCER I ASSUME THERE IS ONLY 1 STREAMER
	// Producer->       HostIP       HostPort     , Consumer->         HostIP
	// HostPort                 StreamToIP   StreamToPort
	std::map<std::pair<std::string, unsigned int>,
	         std::pair<std::pair<std::string, unsigned int>,
	                   std::pair<std::string, unsigned int>>>
	    dataBufferStreamToMap;

	// Visual->HostIP       HostPort
	std::pair<std::string, unsigned int> visualHost;

	for(auto& context : theXDAQContextConfigTree.getChildren())
	{
		// __COUT__ << context.first << __E__;
		for(auto application :
		    context.second.getNode("LinkToApplicationTable").getChildren())
		{
			// __COUT__ << application.first << __E__;
			auto supervisors = application.second.getChildrenMap();
			if(!supervisors[TableViewColumnInfo::COL_NAME_STATUS].getValue<bool>()) continue;
			if(supervisors["Class"].getValue<std::string>() == "ots::FESupervisor" || supervisors["Class"].getValue<std::string>() == "ots::FEDataManagerSupervisor")
			{
				for(auto& frontEndInterface : supervisors["LinkToSupervisorTable"]
				                                  .getNode("LinkToFEInterfaceTable")
				                                  .getChildren())
				{
					// __COUT__ << "FrontEnd!!!" << frontEndInterface.first << __E__;
					auto frontEndInterfaceConfiguration = frontEndInterface.second.getChildrenMap();
					if(frontEndInterfaceConfiguration[TableViewColumnInfo::COL_NAME_STATUS].getValue<bool>() && frontEndInterfaceConfiguration["FEInterfacePluginName"].getValue<std::string>().find("FSSRInterface") != std::string::npos)
					{
						auto frontEndTypeConfiguration =
						    frontEndInterfaceConfiguration["LinkToFETypeTable"]
						        .getChildrenMap();
						// __COUT__ << "StreamToIPAddress: "
						//          << frontEndTypeConfiguration["StreamToIPAddress"]
						//                 .getValue<std::string>()
						//          << __E__;
						// __COUT__ << "StreamToPort: "
						//          << frontEndTypeConfiguration["StreamToPort"]
						//                 .getValue<unsigned int>()
						//          << __E__;
						std::multimap<std::string, unsigned int> channelsMap;
						for(auto& detectorToFEConfiguration :
						    frontEndInterfaceConfiguration["LinkToFEToDetectorTable"]
						        .getChildrenMap())
						{
							// __COUT__ << "FEChannel: "
							//          << detectorToFEConfiguration.second
							//                 .getNode("FEChannel")
							//                 .getValue<unsigned int>()
							//          << __E__;
							auto detectorConfiguration =
							    detectorToFEConfiguration.second
							        .getNode("LinkToDetectorTable")
							        .getChildrenMap();
							// __COUT__ << "DetectorType: "
							//          << detectorConfiguration["DetectorType"]
							//                 .getValue<std::string>()
							//          << __E__;

							if(detectorConfiguration["DetectorType"]
							       .getValue<std::string>() == "FSSR")
							{
								// __COUT__ << "Channel map: " << frontEndInterface.first << "-" << detectorToFEConfiguration.second.getNode("FEChannel").getValue<unsigned int>() << __E__;
								channelsMap.insert(std::pair<std::string, unsigned int>(frontEndInterface.first, detectorToFEConfiguration.second.getNode("FEChannel").getValue<unsigned int>()));
							}
						}
						interfaceStreamToMap[std::make_pair<std::string, unsigned int>(
						    frontEndTypeConfiguration["StreamToIPAddress"]
						        .getValue<std::string>(),
						    frontEndTypeConfiguration["StreamToPort"]
						        .getValue<unsigned int>())] = channelsMap;
					}
				}
			}
			else if(supervisors["Class"].getValue<std::string>() ==
			            "ots::DataManagerSupervisor" ||
			        supervisors["Class"].getValue<std::string>() ==
			            "ots::FEDataManagerSupervisor")
			{
				for(auto& dataManager : supervisors.find("LinkToSupervisorTable")
				                            ->second.getNode("LinkToDataBufferTable")
				                            .getChildren())
				{
					__COUT__ << "Data Manager!!!" << dataManager.first << __E__;

					auto dataManagerConfiguration = dataManager.second.getChildrenMap();
					if(!dataManagerConfiguration[TableViewColumnInfo::COL_NAME_STATUS].getValue<bool>()) continue;

					std::pair<std::string, unsigned int> producerMatch;
					std::pair<std::pair<std::string, unsigned int>,
					          std::pair<std::string, unsigned int>>
					    consumerMatch;
					for(auto& dataBufferGroup :
					    dataManagerConfiguration["LinkToDataProcessorTable"]
					        .getChildren())
					{
						// __COUT__ << "dataBufferGroup!!!" << dataBufferGroup.first
						//          << __E__;
						auto dataProcessors = dataBufferGroup.second.getChildrenMap();
						if(!dataProcessors[TableViewColumnInfo::COL_NAME_STATUS].getValue<bool>()) continue;
						if(dataProcessors["ProcessorPluginName"].getValue<std::string>() == "UDPFSSRDataListenerProducer")
						{
							auto dataProcessorConfiguration =
							    dataProcessors["LinkToProcessorTable"].getChildrenMap();
							// __COUT__ << "HostIPAddress: "
							//          << dataProcessorConfiguration["HostIPAddress"]
							//                 .getValue<std::string>()
							//          << __E__;
							// __COUT__ << "HostPort: "
							//          << dataProcessorConfiguration["HostPort"]
							//                 .getValue<unsigned int>()
							//          << __E__;
							producerMatch = std::make_pair<std::string, unsigned int>(
							    dataProcessorConfiguration["HostIPAddress"]
							        .getValue<std::string>(),
							    dataProcessorConfiguration["HostPort"]
							        .getValue<unsigned int>());
						}
						if(dataProcessors["ProcessorPluginName"].getValue<std::string>() == "UDPFSSRDataStreamerConsumer"
								&& (dataBufferGroup.first == "FSSR0_DB0_DataStreamer0"
									|| dataBufferGroup.first == "FSSR1_DB0_DataStreamer0"
									|| dataBufferGroup.first == "FSSR2_DB0_DataStreamer0"
									|| dataBufferGroup.first == "FSSR3_DB0_DataStreamer0"
									|| dataBufferGroup.first == "FSSR4_DB0_DataStreamer0"
									|| dataBufferGroup.first == "DRS4FSSR0_DB0_DataStreamer0")
						)
						{
							auto dataProcessorConfiguration =
							    dataProcessors["LinkToProcessorTable"].getChildrenMap();
							// __COUT__ << "HostIPAddress: "
							//          << dataProcessorConfiguration["HostIPAddress"]
							//                 .getValue<std::string>()
							//          << __E__;
							// __COUT__ << "HostToPort: "
							//          << dataProcessorConfiguration["HostPort"]
							//                 .getValue<unsigned int>()
							//          << __E__;
							// __COUT__ << "StreamToIPAddress: "
							//          << dataProcessorConfiguration["StreamToIPAddress"]
							//                 .getValue<std::string>()
							//          << __E__;
							// __COUT__ << "StreamToPort: "
							//          << dataProcessorConfiguration["StreamToPort"]
							//                 .getValue<unsigned int>()
							//          << __E__;
							consumerMatch = std::make_pair(
									std::make_pair(
											dataProcessorConfiguration["HostIPAddress"].getValue<std::string>()
											, dataProcessorConfiguration["HostPort"].getValue<unsigned int>()),
											std::make_pair(
													dataProcessorConfiguration["StreamToIPAddress"].getValue<std::string>()
													, dataProcessorConfiguration["StreamToPort"].getValue<unsigned int>()));

							// __COUT__ << "Adding: " << producerMatch.first << "-" << producerMatch.second << "-" << consumerMatch.first.first << "-" << consumerMatch.first.second << __E__;
							dataBufferStreamToMap[producerMatch] = consumerMatch;
						}
					}
				}
			}
			else if(supervisors.find("Class")->second.getValue<std::string>() ==
			        "ots::VisualSupervisor")
			{
				for(auto& dataManager : supervisors.find("LinkToSupervisorTable")
				                            ->second.getNode("LinkToDataBufferTable")
				                            .getChildren())
				{
					// __COUT__ << "Data Manager!!!" << dataManager.first << __E__;

					auto dataManagerConfiguration = dataManager.second.getChildrenMap();
					if(!dataManagerConfiguration[TableViewColumnInfo::COL_NAME_STATUS].getValue<bool>()) continue;
					for(auto& dataBufferGroup: dataManagerConfiguration["LinkToDataProcessorTable"].getChildren())
					{
						// __COUT__ << "dataBufferGroup!!!" << dataBufferGroup.first
						//          << __E__;
						auto dataProcessors = dataBufferGroup.second.getChildrenMap();
						if(!dataProcessors[TableViewColumnInfo::COL_NAME_STATUS].getValue<bool>()) continue;
						if(dataProcessors["ProcessorPluginName"].getValue<std::string>() == "UDPFSSRDataListenerProducer")
						{
							auto dataProcessorConfiguration =
							    dataProcessors["LinkToProcessorTable"].getChildrenMap();
							// __COUT__ << "HostIPAddress: "
							//          << dataProcessorConfiguration["HostIPAddress"]
							//                 .getValue<std::string>()
							//          << __E__;
							// __COUT__ << "HostPort: "
							//          << dataProcessorConfiguration["HostPort"]
							//                 .getValue<unsigned int>()
							//          << __E__;
							visualHost =
							    std::make_pair(dataProcessorConfiguration["HostIPAddress"]
							                       .getValue<std::string>(),
							                   dataProcessorConfiguration["HostPort"]
							                       .getValue<unsigned int>());
						}
					}
				}
			}
		}
	}

	for(auto streamer = dataBufferStreamToMap.begin();
	    streamer != dataBufferStreamToMap.end();
	    streamer++)
	{
		// __COUT__ << streamer->first.first << "-"
		// 		<< streamer->first.second << "-"
		// 		<< streamer->second.first.first << "-"
		// 		<< streamer->second.first.second << "-"
		// 		<< streamer->second.second.first << "="
		// 		<< visualHost.first
		// 		<< " "
		// 		<< streamer->second.second.second
		// 		<< "="
		// 		<< visualHost.second << __E__;
		if(streamer->second.second != visualHost)
			dataBufferStreamToMap.erase(streamer--);
	}
	// for(auto streamer = dataBufferStreamToMap.begin();
	//     streamer != dataBufferStreamToMap.end();
	//     streamer++)
	// {
	// 	__COUT__ << "Survivors: "<< streamer->first.first << "-" << streamer->first.second << "-" << streamer->second.second.second << __E__;
	// }

	myDir_      = myDirectory;
	generalDir_ = myDir_->mkdir("General", "General");
	generalDir_->cd();
	numberOfTriggers_ = new TH1I("NumberOfTriggers", "Number of triggers", 1, -0.5, 0.5);
	planesDir_        = myDir_->mkdir("Planes", "Planes");
	planesDir_->cd();
	std::stringstream name;
	std::stringstream title;
	std::string       ipAddress;
	std::string       port;
	unsigned int      channel;
	for(auto& streamerIt : interfaceStreamToMap)
	{
		//		dataBufferStreamToMap[streamer.first]
		ipAddress = NetworkConverters::nameToStringIP(
		    dataBufferStreamToMap[streamerIt.first].first.first);
		port = NetworkConverters::unsignedToStringPort(
		    dataBufferStreamToMap[streamerIt.first].first.second);
		for(auto& channelIt : streamerIt.second)
		{
			channel = channelIt.second;
			// channel = 0 ;
			// __COUT__ << "Name: " << channelIt.first
			//          << " IP: " << NetworkConverters::stringToNameIP(ipAddress) << ":"
			//          << NetworkConverters::stringToUnsignedPort(port)
			//          << "-> channel: " << channel << __E__;
			if(planeOccupancies_[ipAddress][port].find(channel) ==
			   planeOccupancies_[ipAddress][port].end())
			{
				name.str("");
				title.str("");
				name << channelIt.first << "_Plane" << channel << "_Occupancy";
				title << channelIt.first << " Plane" << channel << " Occupancy";
				planeOccupancies_[ipAddress][port][channel] =
				    new TH1I(name.str().c_str(), title.str().c_str(), 640, 0, 640);
				// __COUT__ << "Added: " << name.str()
				//          << " IP: " << NetworkConverters::stringToNameIP(ipAddress)
				//          << " Port: " << NetworkConverters::stringToUnsignedPort(port)
				//          << " Channel: " << channel << __E__;
				if(channel % 2 == 0)
				{
					name.str("");
					title.str("");
					name << channelIt.first << "_Station" << channel / 2 << "_Profile";
					title << channelIt.first << " Station" << channel / 2 << " Profile";
					// stationProfiles_[ipAddress][port][channel] = new
					// TH2I(name.str().c_str(), title.str().c_str(), 640, 0, 640, 640, 0,
					// 640);
					// __COUT__ << "Added: " << name.str()
					//          << " IP: " << NetworkConverters::stringToNameIP(ipAddress)
					//          << " Port: " << NetworkConverters::stringToUnsignedPort(port)
					//          << " Station: " << channel / 2 << __E__;
				}
			}
		}
	}

	__COUT__ << "Booking done!" << __E__;
}

//========================================================================================================================
void FSSRDQMHistos::clear()
{
	numberOfTriggers_ = 0;
	planeOccupancies_.clear();
}
//========================================================================================================================
void FSSRDQMHistos::fill(std::string& buffer, std::map<std::string, std::string> header)
{
	//__COUT__ << buffer.length() << __E__;
	// int triggerNumber = 0;
	// int triggerHigh = 0;
	// int triggerLow = 0;
	// unsigned long  ipAddress = (((header["IPAddress"][0]&0xff)<<24) +
	// ((header["IPAddress"][1]&0xff)<<16) + ((header["IPAddress"][2]&0xff)<<8) +
	// (header["IPAddress"][3]&0xff)) & 0xffffffff;
	std::string ipAddress = header["IPAddress"];
	std::string port      = header["Port"];

//	__COUT__ << "Got data from IP: " << NetworkConverters::stringToNameIP(ipAddress) <<
//	" port: " << NetworkConverters::stringToUnsignedPort(port) << __E__;

	if (NetworkConverters::stringToUnsignedPort(port) == 48000
			|| NetworkConverters::stringToUnsignedPort(port) == 48001
			|| NetworkConverters::stringToUnsignedPort(port) == 48002
			|| NetworkConverters::stringToUnsignedPort(port) == 48003
			|| NetworkConverters::stringToUnsignedPort(port) == 48004
			)
	{
		buffer = buffer.substr(2);
		theDataDecoder_.convertBuffer(buffer, convertedBuffer_, false);
	}
	else
		theDataDecoder_.convertBuffer(buffer, convertedBuffer_, true);
	//__COUT__ << NetworkConverters::stringToUnsignedPort(port)<< " New buffer" <<
	// __E__;
	unsigned int bufferCounter = 0;
	uint32_t     bco           = 0;
	uint32_t     data          = 0;
	while(!convertedBuffer_.empty())
	{
		if (((NetworkConverters::stringToUnsignedPort(port) == 48000)
				|| (NetworkConverters::stringToUnsignedPort(port) == 48001)
				|| (NetworkConverters::stringToUnsignedPort(port) == 48002)
				|| (NetworkConverters::stringToUnsignedPort(port) == 48003)
				|| (NetworkConverters::stringToUnsignedPort(port) == 48004)
				)
				&& bufferCounter%2 == 1)
		{
			// THIS IS THE BCO
			bco = convertedBuffer_.front();
			//__COUT__ << bco << __E__;
			convertedBuffer_.pop();
			bufferCounter++;
			continue;
		}

		if(theDataDecoder_.isFSSRData(convertedBuffer_.front()))
		{
			//			if(oldData != 0 && oldData==convertedBuffer_.front())
			//				__COUT__ << "There is a copy FSSR: " << std::hex <<
			// convertedBuffer_.front() << " counter: " << bufferCounter << std::dec <<
			// __E__;
			data                       = convertedBuffer_.front();
			FSSRData* detectorDataFSSR = 0;
			theDataDecoder_.decodeData(convertedBuffer_.front(),
			                           (DetectorDataBase**)&detectorDataFSSR);
			if(detectorDataFSSR == 0)
			{
				__COUT__ << "Unrecognized data!" << __E__;
				return;
			}

			if(planeOccupancies_.find(ipAddress) != planeOccupancies_.end() &&
			   planeOccupancies_[ipAddress].find(port) !=
			       planeOccupancies_[ipAddress].end() &&
			   planeOccupancies_[ipAddress][port].find(
			       detectorDataFSSR->getChannelNumber()) !=
			       planeOccupancies_[ipAddress][port].end())
			{
				// if(detectorDataFSSR->getChannelNumber() == 1)
				//{
				//	__COUT__ << detectorDataFSSR->getSensorStrip() << __E__;
				//}
				planeOccupancies_[ipAddress][port][detectorDataFSSR->getChannelNumber()]
				    ->Fill(detectorDataFSSR->getSensorStrip());
			}
			else
				__COUT__ << "ERROR: I haven't book histos for streamer "
				         << NetworkConverters::stringToNameIP(ipAddress)
				         << " port number: "
				         << NetworkConverters::stringToUnsignedPort(port)
				         << " channel: " << detectorDataFSSR->getChannelNumber()
				         << " data: " << std::hex << convertedBuffer_.front() << std::dec
				         << __E__;
			//    }
			//__COUT__ << processorUID_ << " filling histograms!" << __E__;
			//  Float_t px, py, pz;
			//  const Int_t kUPDATE = 1000;
			//   __COUT__ << mthn << "Filling..." << __E__;
			/*
			      for (Int_t i = 1; i <= 1000; i++)
			        {
			          gRandom->Rannor(px, py);
			          pz = px * px + py * py;
			          histo1D_->Fill(px);
			          histo2D_->Fill(px, py);
			          profile_->Fill(px, pz);
			        }
			 */
		}
		else if (theDataDecoder_.isTrigger(convertedBuffer_.front()) && NetworkConverters::stringToUnsignedPort(port) == 48000)
	    {
	        //int offset = 64;//to be subtracted from the trigger time;
			numberOfTriggers_->Fill(0);
			//__COUT__ << "Number of Triggers: " <<
			// theDataDecoder_.decodeTrigger(convertedBuffer_.front()) << __E__;
			// if(triggerNumber != numberOfTriggers_->GetEntries())
			//  __COUT__ << "---------------------------- ERROR: different trigger number
			//  " << triggerNumber << " entries: " << numberOfTriggers_->GetEntries() <<
			//  __E__;
		}
		convertedBuffer_.pop();
		bufferCounter++;
	}
	/*
	      __COUT__ << "base: " <<  typeid(detectorData).name()
	          << "=" << typeid(FSSRData*).name()
	          << " d cast: " << dynamic_cast<FSSRData*>(detectorData)
	          << " r cast: " << static_cast<FSSRData*>(detectorData)
	          << __E__;
	      if (dynamic_cast<FSSRData*>(detectorData) != 0)
	        {
	          FSSRData* data = (FSSRData*)detectorData;
	 */
	//      __COUT__ << "Strip: " << __E__;
	//      __COUT__ << "Strip: " << detectorData->getStripNumber() << __E__;
	//__COUT__ << "Strip: " << detectorData->getChannelNumber() << __E__;
}

//========================================================================================================================
void FSSRDQMHistos::load(std::string fileName)
{
	/*LORE 2016 MUST BE FIXED THIS MONDAY
	DQMHistosBase::openFile (fileName);
	numberOfTriggers_ = (TH1I*)theFile_->Get("General/NumberOfTriggers");

	std::string directory = "Planes";
	std::stringstream name;
	for(unsigned int p=0; p<4; p++)
	{
	    name.str("");
	    name << directory << "/Plane_" << p << "_Occupancy";
	    //FIXME Must organize better all histograms!!!!!
	    //planeOccupancies_.push_back((TH1I*)theFile_->Get(name.str().c_str()));
	}
	//canvas_ = (TCanvas*) theFile_->Get("MainDirectory/MainCanvas");
	//histo1D_ = (TH1F*) theFile_->Get("MainDirectory/Histo1D");
	//histo2D_ = (TH2F*) theFile_->Get("MainDirectory/Histo2D");
	//profile_ = (TProfile*) theFile_->Get("MainDirectory/Profile");
	closeFile();
	 */
}
