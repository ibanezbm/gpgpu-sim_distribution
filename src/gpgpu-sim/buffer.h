#include "link.h"
#include <deque>
#include "mem_fetch.h"

class buffer
{
private:
    bool is_request;
    bool is_push;
    unsigned input;
    unsigned output;
    link_name::link *link_buffer;
    std::deque<mem_fetch*>* queue;
public:
    buffer(link_name::link* link_buffer);
    ~buffer();
    void set_data(bool is_request, bool is_push, unsigned input, unsigned output);
    void push_queue(mem_fetch* mf);
    unsigned size();
    mem_fetch* top();
    mem_fetch* top_back();
    bool empty();
    void pop();
    void cycle(bool is_push, unsigned long cycle);
};