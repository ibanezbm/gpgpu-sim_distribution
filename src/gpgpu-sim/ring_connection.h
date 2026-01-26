#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <tuple>
#include <queue>
#include <unordered_map>
#include "chiplet_wrapper.h"
#include "router.h"

class ring : public ChipletInterconnection {
public:
    ring(unsigned number_of_networks, double freq);
    void push_reply(unsigned input, unsigned output, mem_fetch* mf, unsigned int size, unsigned long cycle);
    void push_request(unsigned input, unsigned output, mem_fetch* mf, unsigned int size, unsigned long cycle);
    mem_fetch* top_reply(unsigned module_number,unsigned long cycle);
    mem_fetch* top_request(unsigned module_number,unsigned long cycle);
    void pop_reply(unsigned module_number,unsigned long cycle);
    void pop_request(unsigned module_number,unsigned long cycle);
    bool has_buffer_reply(unsigned input, unsigned output, unsigned int size);
    bool has_buffer_request(unsigned input, unsigned output, unsigned int size);
    void step(unsigned long cycle);
    void print_stats();
    struct MinObjectComparator {
    bool operator()(const mem_fetch* obj1, const mem_fetch* obj2) const {
        return obj1->get_priority() > obj2->get_priority();
    }
    };
    unsigned links_per_gpu = 0;

private:
    std::unordered_map<unsigned, router*> routers;
    //stats
    std::vector<unsigned long long> moved_bytes_per_chiplet = std::vector<unsigned long long> ();
    std::vector<unsigned long long> messages_per_chiplet = std::vector<unsigned long long> ();
    std::vector<unsigned long long> latency_L2_to_RAM = std::vector<unsigned long long> ();
    std::vector<unsigned long long> latency_RAM_to_L2 = std::vector<unsigned long long> ();
    std::vector<unsigned long long> latency_L2_to_ICNT = std::vector<unsigned long long> ();
    std::vector<unsigned long long> messages_per_chiplet_stats = std::vector<unsigned long long> ();
};