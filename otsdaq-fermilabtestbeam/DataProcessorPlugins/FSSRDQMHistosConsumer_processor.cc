#include "otsdaq-fermilabtestbeam/DataProcessorPlugins/FSSRDQMHistosConsumer.h"
#include "otsdaq/MessageFacility/MessageFacility.h"
#include "otsdaq/Macros/CoutMacros.h"
#include "otsdaq/Macros/ProcessorPluginMacros.h"
#include <TDirectory.h>
#include <TFile.h>
//#include <TROOT.h>

#include <unistd.h>

using namespace ots;

//========================================================================================================================
FSSRDQMHistosConsumer::FSSRDQMHistosConsumer(
    std::string              supervisorApplicationUID,
    std::string              bufferUID,
    std::string              processorUID,
    const ConfigurationTree& theXDAQContextConfigTree,
    const std::string&       configurationPath)
    : WorkLoop(processorUID)
    , DQMHistosConsumerBase(
          supervisorApplicationUID, bufferUID, processorUID, LowConsumerPriority)
    , Configurable(theXDAQContextConfigTree, configurationPath)
    , filePath_(theXDAQContextConfigTree.getNode(configurationPath)
                    .getNode("FilePath")
                    .getValue<std::string>())
    , fileRadix_(theXDAQContextConfigTree.getNode(configurationPath)
                     .getNode("RadixFileName")
                     .getValue<std::string>())
    , saveFile_(theXDAQContextConfigTree.getNode(configurationPath)
                    .getNode("SaveFile")
                    .getValue<bool>())

{
	//	gStyle->SetPalette(1);
}

//========================================================================================================================
FSSRDQMHistosConsumer::~FSSRDQMHistosConsumer(void) { DQMHistosBase::closeFile(); }
//========================================================================================================================
void FSSRDQMHistosConsumer::startProcessingData(std::string runNumber)
{
	if(saveFile_)
	{
		DQMHistosBase::openFile(filePath_ + "/" + fileRadix_ + "_Run" + runNumber +
		                        ".root");
	}
	DQMHistosBase::myDirectory_ =
	    DQMHistosBase::theFile_->mkdir("StripTelescope", "StripTelescope");
	DQMHistosBase::myDirectory_->cd();
	FSSRDQMHistos::book(DQMHistosBase::myDirectory_,
	                    Configurable::theXDAQContextConfigTree_,
	                    Configurable::theConfigurationPath_);

	DataConsumer::startProcessingData(runNumber);
}

//========================================================================================================================
void FSSRDQMHistosConsumer::stopProcessingData(void)
{
	DataConsumer::stopProcessingData();
	if(saveFile_)
	{
		DQMHistosBase::save();
	}
	closeFile();
}

//========================================================================================================================
bool FSSRDQMHistosConsumer::workLoopThread(toolbox::task::WorkLoop* workLoop)
{
	// std::cout << __COUT_HDR_FL__ << __PRETTY_FUNCTION__ << DataProcessor::processorUID_
	// << " running, because workloop: " << WorkLoop::continueWorkLoop_ << std::endl;
	fastRead();
	return WorkLoop::continueWorkLoop_;
}

//========================================================================================================================
void FSSRDQMHistosConsumer::fastRead(void)
{
	if(DataConsumer::read(dataP_, headerP_) < 0)
	{
		usleep(100);
		return;
	}
	FSSRDQMHistos::fill(*dataP_, *headerP_);
	DataConsumer::setReadSubBuffer<std::string, std::map<std::string, std::string>>();
}

DEFINE_OTS_PROCESSOR(FSSRDQMHistosConsumer)
