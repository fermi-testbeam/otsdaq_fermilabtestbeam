#ifndef OTSDAQ_FERMILABTESTBEAM_ARTMODULES_DETAIL_DATAREQUESTMESSAGE_HH
#define OTSDAQ_FERMILABTESTBEAM_ARTMODULES_DETAIL_DATAREQUESTMESSAGE_HH

namespace otsftbf
{
struct DataRequestMessage
{
	uint32_t eventNumber;
	bool     wantMWPC;
	bool     wantSTIB;
	bool     wantCherenkov;
	bool     wantUDP;
	uint64_t request_magic;

	DataRequestMessage()
	    : eventNumber(0)
	    , wantMWPC(false)
	    , wantSTIB(false)
	    , wantCherenkov(false)
	    , wantUDP(false)
	    , request_magic(0)
	{
	}
	DataRequestMessage(
	    uint32_t e, bool m = false, bool s = false, bool c = false, bool u = false)
	    : eventNumber(e)
	    , wantMWPC(m)
	    , wantSTIB(s)
	    , wantCherenkov(c)
	    , wantUDP(u)
	    , request_magic(0xAAAABBBBCCCCDDDD)
	{
	}
	bool isValid() const { return request_magic == 0xAAAABBBBCCCCDDDD; }
};

struct DataResponseHeader
{
	uint64_t response_fragment_count;
	uint64_t response_magic;
	uint64_t first_event_in_buffer;
	uint64_t last_event_in_buffer;

	DataResponseHeader()
	    : response_fragment_count(0)
	    , response_magic(0)
	    , first_event_in_buffer(0)
	    , last_event_in_buffer(0)
	{
	}
	DataResponseHeader(uint64_t count, uint64_t firstEvent, uint64_t lastEvent)
	    : response_fragment_count(count)
	    , response_magic(0xABCDABCDABCDABCD)
	    , first_event_in_buffer(firstEvent)
	    , last_event_in_buffer(lastEvent)
	{
	}
	bool isValid() const { return response_magic == 0xABCDABCDABCDABCD; }
};
}  // namespace otsftbf

#endif  // OTSDAQ_FERMILABTESTBEAM_ARTMODULES_DETAIL_DATAREQUESTMESSAGE_HH