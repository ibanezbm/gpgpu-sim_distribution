#include "mem_fetch.h"
#include <deque>
#include "buffer.h"

buffer::buffer(link_name::link* link_buffer){
    this->link_buffer = link_buffer;
    this->queue = new std::deque<mem_fetch*>();
}

void buffer::set_data(bool is_request, bool is_push, unsigned input, unsigned output){
    this->is_request = is_request;
    this->is_push = is_push;
    this->input = input;
    this->output = output;
}

void buffer::push_queue(mem_fetch* mf){
    this->queue->push_back(mf);
}

unsigned buffer::size(){
    return this->queue->size();
}

mem_fetch* buffer::top(){
    return this->queue->front();
}

mem_fetch* buffer::top_back(){
    return this->queue->back();
}

bool buffer::empty(){
    return this->queue->empty();
}

void buffer::pop(){
    this->queue->pop_front();
}

void buffer::cycle(bool is_push, unsigned long cycle){
    if(is_push && !this->queue->empty()){
        mem_fetch* mf = this->queue->front();
        if(mf){
            this->link_buffer->push(mf);
            this->queue->pop_front();
        }
        this->link_buffer->cycle(cycle);
    }else if(!is_push && !this->link_buffer->empty()){
        mem_fetch* mf = this->link_buffer->top();
        if(mf){
            if(mf->get_priority()<=cycle){
                this->queue->push_back(mf);
                this->link_buffer->pop();
            }
        }
    }
}


