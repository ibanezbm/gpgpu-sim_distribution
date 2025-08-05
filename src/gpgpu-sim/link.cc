#include "link.h"

link_name::link::link(){
    this->queue = new std::deque<mem_fetch*>();
}
link_name::link::~link(){}

void link_name::link::cycle(unsigned long cycle){
    if (last_cycle != cycle){
        last_cycle = cycle;
    }
}
void link_name::link::push(mem_fetch* mf){
    this->queue->push_back(mf);
}

mem_fetch* link_name::link::top(){
    return this->queue->front();
}

void link_name::link::pop(){
    this->queue->pop_front();
}

unsigned link_name::link::empty(){
    return this->queue->empty();
}
