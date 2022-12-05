#ifndef _ots_FECAMACTCPInterface_h_
#define _ots_FECAMACTCPInterface_h_

#include "otsdaq/FECore/FEVInterface.h"
#include "otsdaq/NetworkUtilities/TCPPublishServer.h"

class UsbHandler;
#include <fstream>
#include <iostream>
#include <string>

namespace ots {
	class FECAMACTCPInterface : public ots::FEVInterface, public ots::TCPPublishServer
	{

	public:
		FECAMACTCPInterface(const std::string& interfaceUID, const ots::ConfigurationTree& theXDAQContextConfigTree, const std::string& interfaceConfigurationPath);
		virtual ~FECAMACTCPInterface(void);

		void configure(void);
		void halt(void);
		void pause(void);
		void resume(void);
		void start(std::string runNumber = "");
		void stop(void);
		bool running(void);

		void universalRead(char *address, char *returnValue) override { ; }
		void universalWrite(char *address, char *writeValue) override { ; }

	private:
		std::unique_ptr<UsbHandler> camac;
		std::ofstream               output;
		std::string                 rawOutputFile;
		std::string                 cfgHeader;
		std::string                 cardList;
		bool                        rawOutput;
		bool                        init_sent_;
	};

}

#endif
