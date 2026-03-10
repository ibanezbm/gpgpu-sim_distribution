#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <tuple>
#include <queue>
#include <unordered_map>
#include "mem_fetch.h"
#include "../option_parser.h"

#ifndef CHIPLET_WRAPPER_H
#define CHIPLET_WRAPPER_H

class ChipletInterconnection {
public:
    ChipletInterconnection() = default;
    virtual ~ChipletInterconnection() = default;
    virtual void Init() = 0;
    virtual void push_reply(unsigned input, unsigned output, mem_fetch* mf, unsigned int size, unsigned long cycle) = 0;
    virtual void push_request(unsigned input, unsigned output, mem_fetch* mf, unsigned int size, unsigned long cycle) = 0;
    virtual mem_fetch* top_reply(unsigned module_number, unsigned long cycle) = 0;
    virtual mem_fetch* top_request(unsigned module_number, unsigned long cycle) = 0;
    virtual void pop_reply(unsigned module_number, unsigned long cycle) = 0;
    virtual void pop_request(unsigned module_number, unsigned long cycle) = 0;
    virtual bool has_buffer_reply(unsigned input, unsigned output, unsigned int size) = 0;
    virtual bool has_buffer_request(unsigned input, unsigned output, unsigned int size) = 0;
    virtual void step(unsigned long cycle) = 0;
    virtual void print_stats() = 0;
    unsigned number_of_chiplets = 0;
    double chiplet_frequency = 0.0;
    unsigned long long cycles = 0;
};

enum chiplet_mode { CHIPLET_INTERSIM = 1, CHIPLET_RING = 2 };

void chiplet_reg_options(class OptionParser* opp);
ChipletInterconnection* chiplet_wrapper_init(int number_of_nodes, double chiplet_freq);

#endif
