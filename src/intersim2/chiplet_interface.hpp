#ifndef _CHIPLET_INTERFACE_HPP_
#define _CHIPLET_INTERFACE_HPP_

#include <vector>
#include <queue>
#include <iostream>
#include <map>
#include "../gpgpu-sim/chiplet_wrapper.h"
#include "interconnect_interface.hpp"
#include "intersim_config.hpp"

using namespace std;

class ChipletInterface : public ChipletInterconnection, InterconnectInterface {
    public:
        ChipletInterface(unsigned number_of_networks, char* g_chiplet_config_filename);
        virtual ~ChipletInterface() override;

        void push_reply(unsigned input, unsigned output, mem_fetch* mf, unsigned int size, unsigned long cycle) override;
        void push_request(unsigned input, unsigned output, mem_fetch* mf, unsigned int size, unsigned long cycle) override;
        mem_fetch* top_reply(unsigned module_number, unsigned long cycle) override;
        mem_fetch* top_request(unsigned module_number, unsigned long cycle) override;
        void pop_reply(unsigned module_number, unsigned long cycle) override;
        void pop_request(unsigned module_number, unsigned long cycle) override;
        bool has_buffer_reply(unsigned input, unsigned output, unsigned int size) override;
        bool has_buffer_request(unsigned input, unsigned output, unsigned int size) override;
        void step(unsigned long cycle) override;
        void print_stats() override;
        void CreateInterconnect(unsigned n_nodes);
};

#endif