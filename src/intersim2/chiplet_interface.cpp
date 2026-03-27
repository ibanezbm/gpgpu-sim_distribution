#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <utility>
#include <algorithm>

#include "interconnect_interface.hpp"
#include "../gpgpu-sim/mem_fetch.h"
#include "../gpgpu-sim/chiplet_wrapper.h"
#include "chiplet_interface.hpp"
#include "routefunc.hpp"
#include "globals.hpp"
#include "trafficmanager.hpp"
#include "flit.hpp"
#include "gputrafficmanager.hpp"
#include "booksim.hpp"
#include "intersim_config.hpp"
#include "network.hpp"
#include "interconnect_interface.hpp"
#include "../trace.h"

ChipletInterface::ChipletInterface(unsigned number_of_networks, char* g_chiplet_config_filename, double chiplet_freq)
    : ChipletInterconnection()
    , InterconnectInterface(chiplet_freq)
{
    number_of_chiplets = number_of_networks;
    _icnt_config = new IntersimConfig();
    _icnt_config->ParseFile(g_chiplet_config_filename);
    this->CreateInterconnect(number_of_networks);
    _traffic_manager->Init();
}

void ChipletInterface::Init() {
  _traffic_manager->Init();
}

void ChipletInterface::push_reply(unsigned input, unsigned output, mem_fetch* mf, unsigned int size, unsigned long cycle) { 
    // it should have free buffer
  assert(has_buffer_reply(input, output, size));

  DPRINTF(INTERCONNECT, "Sent %d bytes from %d to %d", size, input, output);
  
  int output_icntID = _node_map[output];
  int input_icntID = _node_map[input];

#if 0
  cout<<"Call interconnect push input: "<<input<<" output: "<<output<<endl;
#endif

  //TODO: move to _IssuePacket
  //TODO: create a Inject and wrap _IssuePacket and _GeneratePacket
  unsigned int n_flits = size / _flit_size + ((size % _flit_size)? 1:0);
  int subnet;
  if(_flit_size < 256 && n_flits == 1){
    n_flits = 256 / _flit_size; 
  }
  
  subnet = 1;

  //TODO: Remove mem_fetch to reduce dependency
  Flit::FlitType packet_type;

  switch (mf->get_type()) {
    case READ_REQUEST:  packet_type = Flit::READ_REQUEST   ;break;
    case WRITE_REQUEST: packet_type = Flit::WRITE_REQUEST  ;break;
    case READ_REPLY:    packet_type = Flit::READ_REPLY     ;break;
    case WRITE_ACK:     packet_type = Flit::WRITE_REPLY    ;break;
    case FINISH_REMOTE: packet_type = Flit::READ_REPLY    ;break;
    default:
    	{
    		cout<<"Type "<<mf->get_type()<<" is undefined!"<<endl;
    		assert (0 && "Type is undefined");
    	}
  }

  _traffic_manager->GeneratePacket(input_icntID, -1, 0 /*class*/, _traffic_manager->getTime(), subnet, n_flits, packet_type, static_cast<void*>(mf), output_icntID);

#if DOUB
  cout <<"Traffic[" << subnet << "] (mapped) sending form "<< input_icntID << " to " << output_icntID << endl;
#endif
}

void ChipletInterface::push_request(unsigned input, unsigned output, mem_fetch* mf, unsigned int size, unsigned long cycle) { 
  // it should have free buffer
  assert(has_buffer_request(input, output, size));

  DPRINTF(INTERCONNECT, "Sent %d bytes from %d to %d", size, input, output);
  
  int output_icntID = _node_map[output];
  int input_icntID = _node_map[input];

#if 0
  cout<<"Call interconnect push input: "<<input<<" output: "<<output<<endl;
#endif

  //TODO: move to _IssuePacket
  //TODO: create a Inject and wrap _IssuePacket and _GeneratePacket
  unsigned int n_flits = size / _flit_size + ((size % _flit_size)? 1:0);
  if(_flit_size < 256 && n_flits == 1){
    n_flits = 256 / _flit_size; 
  }
  int subnet;
  
  subnet = 0;

  //TODO: Remove mem_fetch to reduce dependency
  Flit::FlitType packet_type;

  switch (mf->get_type()) {
    case READ_REQUEST:  packet_type = Flit::READ_REQUEST   ;break;
    case WRITE_REQUEST: packet_type = Flit::WRITE_REQUEST  ;break;
    case READ_REPLY:    packet_type = Flit::READ_REPLY     ;break;
    case WRITE_ACK:     packet_type = Flit::WRITE_REPLY    ;break;
    case TO_SM:         packet_type = Flit::READ_REQUEST       ;break;
    default:
    	{
    		cout<<"Type "<<mf->get_type()<<" is undefined!"<<endl;
    		assert (0 && "Type is undefined");
    	}
  }

  //TODO: _include_queuing ?
  _traffic_manager->GeneratePacket(input_icntID, -1, 0 /*class*/, _traffic_manager->getTime(), subnet, n_flits, packet_type, static_cast<void*>(mf), output_icntID);

#if DOUB
  cout <<"Traffic[" << subnet << "] (mapped) sending form "<< input_icntID << " to " << output_icntID << endl;
#endif
}

mem_fetch* ChipletInterface::top_reply(unsigned module_number, unsigned long cycle) { 

    int icntID = _node_map[module_number];
#if 0
  cout<<"Call interconnect POP  " << output<<endl;
#endif

  void* data = NULL;

  int subnet = 1;

  int turn = _round_robin_turn[subnet][icntID];
  for (int vc=0;(vc<_vcs) && (data==NULL);vc++) {
    if (_boundary_buffer[subnet][icntID][turn].HasPacket()) {
      data = _boundary_buffer[subnet][icntID][turn].TopPacket();
    }
    turn++;
    if (turn == _vcs) turn = 0;
  }

  return static_cast<mem_fetch*>(data);

}

mem_fetch* ChipletInterface::top_request(unsigned module_number, unsigned long cycle) { 

  int icntID = _node_map[module_number];
#if 0
  cout<<"Call interconnect POP  " << output<<endl;
#endif

  void* data = NULL;

  int subnet = 0;

  int turn = _round_robin_turn[subnet][icntID];
  for (int vc=0;(vc<_vcs) && (data==NULL);vc++) {
    if (_boundary_buffer[subnet][icntID][turn].HasPacket()) {
      data = _boundary_buffer[subnet][icntID][turn].TopPacket();
    }
    turn++;
    if (turn == _vcs) turn = 0;
  }

  return static_cast<mem_fetch*>(data);
 }

void ChipletInterface::pop_reply(unsigned module_number, unsigned long cycle) { 
    
  int icntID = _node_map[module_number];
#if 0
  cout<<"Call interconnect POP  " << output<<endl;
#endif

  void* data = NULL;
  int subnet = 1;

  int turn = _round_robin_turn[subnet][icntID];
  for (int vc=0;(vc<_vcs) && (data==NULL);vc++) {
    if (_boundary_buffer[subnet][icntID][turn].HasPacket()) {
      data = _boundary_buffer[subnet][icntID][turn].PopPacket();
    }
    turn++;
    if (turn == _vcs) turn = 0;
  }
  if (data) {
    _round_robin_turn[subnet][icntID] = turn;
  }

  return;
}
void ChipletInterface::pop_request(unsigned module_number, unsigned long cycle) { 
    
  int icntID = _node_map[module_number];
#if 0
  cout<<"Call interconnect POP  " << output<<endl;
#endif

  void* data = NULL;

  // 0-_n_shader-1 indicates reply(network 1), otherwise request(network 0)
  int subnet = 0;

  int turn = _round_robin_turn[subnet][icntID];
  for (int vc=0;(vc<_vcs) && (data==NULL);vc++) {
    if (_boundary_buffer[subnet][icntID][turn].HasPacket()) {
      data = _boundary_buffer[subnet][icntID][turn].PopPacket();
    }
    turn++;
    if (turn == _vcs) turn = 0;
  }
  if (data) {
    _round_robin_turn[subnet][icntID] = turn;
  }
  return;
}

bool ChipletInterface::has_buffer_reply(unsigned input, unsigned output, unsigned int size) { 

  bool has_buffer = false;
  unsigned int n_flits = size / _flit_size + ((size % _flit_size)? 1:0);
  int icntID = _node_map.find(input)->second;
  has_buffer = _traffic_manager->getInputQueueSize(1, icntID, 0) + n_flits <= _input_buffer_capacity;

  return has_buffer;

}
bool ChipletInterface::has_buffer_request(unsigned input, unsigned output, unsigned int size) { 
  bool has_buffer = false;
  unsigned int n_flits = size / _flit_size + ((size % _flit_size)? 1:0);
  int icntID = _node_map.find(input)->second;
  has_buffer = _traffic_manager->getInputQueueSize(0, icntID, 0) + n_flits <= _input_buffer_capacity;
  return has_buffer;
}
void ChipletInterface::step(unsigned long cycle) { 

  _traffic_manager->Step();
  cycles++;

}
void ChipletInterface::print_stats() {
  _traffic_manager->setDrainTime(_traffic_manager->getTime());
  // hack: also _total_sims equals to number of kernel calls
  _traffic_manager->incrementTotalSims();
  _traffic_manager->UpdateStats();
  _traffic_manager->UpdateOverallStats();
  _traffic_manager->DisplayStats();
  _traffic_manager->DisplayOverallStats();
 }

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
    _traffic_manager->set_chiplet_network(true);
    
    for (int i = 0; i < _subnets; ++i) {
      for (size_t j = 0; j < _net[i]->_eject.size(); j++) {
        _net[i]->_eject[j]->_tm = _traffic_manager;
        _net[i]->_eject_cred[j]->_tm = _traffic_manager;
      }
      for (size_t j = 0; j < _net[i]->_inject.size(); j++) {
        _net[i]->_inject[j]->_tm = _traffic_manager;
        _net[i]->_inject_cred[j]->_tm = _traffic_manager;
      }
      for (size_t j = 0; j < _net[i]->_chan.size(); j++) {
         _net[i]->_chan[j]->_tm = _traffic_manager;
         _net[i]->_chan_cred[j]->_tm = _traffic_manager;
      }
      for (size_t j = 0; j < _net[i]->_routers.size(); j++) {
         _net[i]->_routers[j]->_tm = _traffic_manager;
      }
    }

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
    _CreateNodeMap(0, 0, _traffic_manager->getNodeCount(), _icnt_config->GetInt("use_map"));
}
