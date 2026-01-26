#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <utility>
#include <algorithm>

#include "interconnect_interface.hpp"
#include "mem_fetch.h"
#include "chiplet_wrapper.h"
#include "chiplet_interface.hpp"
#include "routefunc.hpp"
#include "globals.hpp"
#include "trafficmanager.hpp"
#include "flit.hpp"
#include "gputrafficmanager.hpp"
#include "booksim.hpp"
#include "intersim_config.hpp"
#include "network.hpp"
#include "trace.h"

ChipletInterface::ChipletInterface(unsigned number_of_networks, char* g_chiplet_config_filename)
    : ChipletInterconnection()
    , InterconnectInterface()
{
    _icnt_config = new IntersimConfig();
    _icnt_config->ParseFile(g_chiplet_config_filename);
    this->CreateInterconnect(number_of_networks);

}

ChipletInterface::~ChipletInterface()
{
    this->~InterconnectInterface();
};

void ChipletInterface::push_reply(unsigned input, unsigned output, mem_fetch* mf, unsigned int size, unsigned long cycle) { 
    // it should have free buffer
  assert(HasBuffer(input_deviceID, size));

  DPRINTF(INTERCONNECT, "Sent %d bytes from %d to %d", size, input_deviceID, output_deviceID);
  
  int output_icntID = _node_map[output_deviceID];
  int input_icntID = _node_map[input_deviceID];

#if 0
  cout<<"Call interconnect push input: "<<input<<" output: "<<output<<endl;
#endif

  //TODO: move to _IssuePacket
  //TODO: create a Inject and wrap _IssuePacket and _GeneratePacket
  unsigned int n_flits = size / _flit_size + ((size % _flit_size)? 1:0);
  int subnet;
  
  subnet = 1;

  //TODO: Remove mem_fetch to reduce dependency
  Flit::FlitType packet_type;
  mem_fetch* mf = static_cast<mem_fetch*>(data);

  switch (mf->get_type()) {
    case READ_REQUEST:  packet_type = Flit::READ_REQUEST   ;break;
    case WRITE_REQUEST: packet_type = Flit::WRITE_REQUEST  ;break;
    case READ_REPLY:    packet_type = Flit::READ_REPLY     ;break;
    case WRITE_ACK:     packet_type = Flit::WRITE_REPLY    ;break;
    default:
    	{
    		cout<<"Type "<<mf->get_type()<<" is undefined!"<<endl;
    		assert (0 && "Type is undefined");
    	}
  }

  //TODO: _include_queuing ?
  _traffic_manager->_GeneratePacket( input_icntID, -1, 0 /*class*/, _traffic_manager->_time, subnet, n_flits, packet_type, data, output_icntID);

#if DOUB
  cout <<"Traffic[" << subnet << "] (mapped) sending form "<< input_icntID << " to " << output_icntID << endl;
#endif
}
void ChipletInterface::push_request(unsigned input, unsigned output, mem_fetch* mf, unsigned int size, unsigned long cycle) { 
        // it should have free buffer
  assert(HasBuffer(input_deviceID, size));

  DPRINTF(INTERCONNECT, "Sent %d bytes from %d to %d", size, input_deviceID, output_deviceID);
  
  int output_icntID = _node_map[output_deviceID];
  int input_icntID = _node_map[input_deviceID];

#if 0
  cout<<"Call interconnect push input: "<<input<<" output: "<<output<<endl;
#endif

  //TODO: move to _IssuePacket
  //TODO: create a Inject and wrap _IssuePacket and _GeneratePacket
  unsigned int n_flits = size / _flit_size + ((size % _flit_size)? 1:0);
  int subnet;
  
  subnet = 0;

  //TODO: Remove mem_fetch to reduce dependency
  Flit::FlitType packet_type;
  mem_fetch* mf = static_cast<mem_fetch*>(data);

  switch (mf->get_type()) {
    case READ_REQUEST:  packet_type = Flit::READ_REQUEST   ;break;
    case WRITE_REQUEST: packet_type = Flit::WRITE_REQUEST  ;break;
    case READ_REPLY:    packet_type = Flit::READ_REPLY     ;break;
    case WRITE_ACK:     packet_type = Flit::WRITE_REPLY    ;break;
    default:
    	{
    		cout<<"Type "<<mf->get_type()<<" is undefined!"<<endl;
    		assert (0 && "Type is undefined");
    	}
  }

  //TODO: _include_queuing ?
  _traffic_manager->_GeneratePacket( input_icntID, -1, 0 /*class*/, _traffic_manager->_time, subnet, n_flits, packet_type, data, output_icntID);

#if DOUB
  cout <<"Traffic[" << subnet << "] (mapped) sending form "<< input_icntID << " to " << output_icntID << endl;
#endif
}

mem_fetch* ChipletInterface::top_reply(unsigned, unsigned long) { return nullptr; }
mem_fetch* ChipletInterface::top_request(unsigned, unsigned long) { return nullptr; }

void ChipletInterface::pop_reply(unsigned, unsigned long) { }
void ChipletInterface::pop_request(unsigned, unsigned long) { }

bool ChipletInterface::has_buffer_reply(unsigned, unsigned, unsigned int) { return false; }
bool ChipletInterface::has_buffer_request(unsigned, unsigned, unsigned int) { return false; }

void ChipletInterface::step(unsigned long) { }
void ChipletInterface::print_stats() { }

void ChipletInterface::CreateInterconnect(unsigned n_nodes){
    RoutingContext* rc = InitializeRoutingMap(*_icnt_config);

    gPrintActivity = (_icnt_config->GetInt("print_activity") > 0);
    gTrace = (_icnt_config->GetInt("viewer_trace") > 0);

    string watch_out_file = _icnt_config->GetStr( "watch_out" );
    if(watch_out_file == "") {
        gWatchOut = NULL;
    } else if(watch_out_file == "-") {
        gWatchOut = &cout;
    } else {
        gWatchOut = new ofstream(watch_out_file.c_str());
    }

    _subnets = _icnt_config->GetInt("subnets");
    assert(_subnets);

    /*To include a new network, must register the network here
    *add an else if statement with the name of the network
    */
    _net.resize(_subnets);
    for (int i = 0; i < _subnets; ++i) {
        ostringstream name;
        name << "network_" << i;
        _net[i] = Network::New( *_icnt_config, name.str(), rc);
    }

    assert(_icnt_config->GetStr("sim_type") == "gpgpusim");
    _traffic_manager = static_cast<GPUTrafficManager*>(TrafficManager::New( *_icnt_config, _net, this, rc)) ;

    _flit_size = _icnt_config->GetInt( "flit_size" );

    // Config for interface buffers
    if (_icnt_config->GetInt("ejection_buffer_size")) {
        _ejection_buffer_capacity = _icnt_config->GetInt( "ejection_buffer_size" ) ;
    } else {
        _ejection_buffer_capacity = _icnt_config->GetInt( "vc_buf_size" );
    }

    _boundary_buffer_capacity = _icnt_config->GetInt( "boundary_buffer_size" ) ;
    assert(_boundary_buffer_capacity);
    if (_icnt_config->GetInt("input_buffer_size")) {
        _input_buffer_capacity = _icnt_config->GetInt("input_buffer_size");
    } else {
        _input_buffer_capacity = 9;
    }
    _vcs = _icnt_config->GetInt("num_vcs");

    _CreateBuffer();
}
