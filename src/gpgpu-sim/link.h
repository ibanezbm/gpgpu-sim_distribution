#include <queue>
#include <vector>
#include "mem_fetch.h"

namespace link_name{
    class link {
    public:
        struct MinObjectComparator {
            bool operator()(const mem_fetch* obj1, const mem_fetch* obj2) const {
                return obj1->get_priority() > obj2->get_priority();
            }
        };
        link();
        ~link();
        void push(mem_fetch* mf);
        void cycle(unsigned long cycle);
        mem_fetch* top();
        void pop();
        unsigned empty();
        
    private:
        unsigned long last_cycle = 1;
        std::deque<mem_fetch*>* queue;

    };
}