#include "router.h"
#include <cmath>

router::router(unsigned number_of_networks, double freq){
    this->number_of_networks = number_of_networks;
    this->frequency = freq;
    if(number_of_networks == 2){
        this->links_per_gpu = 2;
    }else{
        this->links_per_gpu = 4;
    }
}

router::~router(){}

void router::add_buffer(bool is_request, bool is_push, unsigned input, unsigned output, buffer* buffer){
    buffer->set_data(is_request,is_push, input, output);
    this->buffers[std::make_tuple(is_request, is_push, input, output)] = buffer;
}

void router::push_request(unsigned input, unsigned output, mem_fetch* mf, unsigned int size, unsigned long cycle){
    std::srand(static_cast<unsigned int>(0));
    //int random_number = std::rand() % 2;
    if((output - input + number_of_networks) % number_of_networks > number_of_networks / 2){
        output = (input - 1 + number_of_networks) % number_of_networks;
    }else{
        output = (input + 1) % number_of_networks;
    }
    /*
    if((input == 0 && output == 2)){
        output = random_number ? 1 : 3;
    }else if(((input == 2 && output == 0))){
        output = random_number ? 1 : 3;
    }else if((input == 1 && output == 3)){
        output = random_number ? 0 : 2;
    }else if((input == 3 && output == 1)){
        output = random_number ? 0 : 2;
    }*/
    double bw = BW_PER_LINK / (frequency/1000000/links_per_gpu) ;
    if(!this->buffers[std::make_tuple(true, true, input, output)]->empty()){
        unsigned long long back = this->buffers[std::make_tuple(true, true, input, output)]->top_back()->get_priority();
        mf->set_priority(back+32+((int)ceil(size/bw)));
        //mf->set_priority(back+32+size/8);
    }else{
        mf->set_priority(cycle+32+((int)ceil(size/bw)));
        //mf->set_priority(cycle+32+size/8);
    }
    this->buffers[std::make_tuple(true, true, input, output)]->push_queue(mf);
}

void router::push_reply(unsigned input, unsigned output, mem_fetch* mf, unsigned int size, unsigned long cycle){
    std::srand(static_cast<unsigned int>(0));
    //int random_number = std::rand() % 2;
    if((output - input + number_of_networks) % number_of_networks > number_of_networks / 2){
        output = (input - 1 + number_of_networks) % number_of_networks;
    }else{
        output = (input + 1) % number_of_networks;
    }
    /*
    if((input == 0 && output == 2)){
        output = random_number ? 1 : 3;
    }else if(((input == 2 && output == 0))){
        output = random_number ? 1 : 3;
    }else if((input == 1 && output == 3)){
        output = random_number ? 0 : 2;
    }else if((input == 3 && output == 1)){
        output = random_number ? 0 : 2;
    }*/
    double bw = BW_PER_LINK / (frequency/1000000/links_per_gpu) ;
    unsigned long long back = 0;
    if(!this->buffers[std::make_tuple(false, true, input, output)]->empty()){
        back = this->buffers[std::make_tuple(false, true, input, output)]->top_back()->get_priority();
        mf->set_priority(back+32+((int)ceil(size/bw)));
        //mf->set_priority(back+32+size/8);
    }else{
        mf->set_priority(cycle+32+((int)ceil(size/bw)));
        //mf->set_priority(cycle+32+size/8);
    }
    this->buffers[std::make_tuple(false, true, input, output)]->push_queue(mf);
}

bool router::has_buffer_reply(unsigned input, unsigned output, unsigned int size){
    if(this->buffers.find(std::make_tuple(false, true, input, output))==this->buffers.end()){
        return this->buffers[std::make_tuple(false, true, input, (input+1)%number_of_networks)]->size() < 64 ||
                this->buffers[std::make_tuple(false, true, input, (input-1)%number_of_networks)]->size() < 64;
    }
    return this->buffers[std::make_tuple(false, true, input, output)]->size() < 64;
}
bool router::has_buffer_request(unsigned input, unsigned output, unsigned int size){
    if(this->buffers.find(std::make_tuple(true, true, input, output))==this->buffers.end()){
        return this->buffers[std::make_tuple(true, true, input, (input+1)%number_of_networks)]->size() < 64 ||
                this->buffers[std::make_tuple(true, true, input, (input-1)%number_of_networks)]->size() < 64;
    }
    return this->buffers[std::make_tuple(true, true, input, output)]->size() < 64;
}

mem_fetch* router::top_reply(unsigned module_number, unsigned long cycle){
    bool first_buffer = this->first_buffer;
    this->first_buffer ? this->first_buffer = false : this->first_buffer = true;
    mem_fetch* mf = NULL;
    for (auto& par : buffers) {
        if(module_number == std::get<3>(par.first) && 
            !std::get<0>(par.first) && !std::get<1>(par.first)){
            if (!first_buffer){ first_buffer = true; continue;}
            if(!par.second->empty()){
                mf = par.second->top();
                if (mf->get_priority() <= cycle){
                    return mf;
                }else{
                    mf = NULL;
                }
            }    
        }
    }
    return mf;
}

mem_fetch* router::top_request(unsigned module_number, unsigned long cycle){
    bool first_buffer = this->first_buffer;
    this->first_buffer ? this->first_buffer = false : this->first_buffer = true;
    mem_fetch* mf = NULL;
    for (auto& par : buffers) {
        if(module_number == std::get<3>(par.first) && 
            std::get<0>(par.first) && !std::get<1>(par.first)){
            if (!first_buffer){ first_buffer = true; continue;}
            if(!par.second->empty()){
                mf = par.second->top();
                if (mf->get_priority() <= cycle){
    //printf("MF wtop uid:%d warp:%d tpc:%d, pc:%d\n", mf->get_request_uid(), mf->get_wid(), mf->get_tpc(), mf->get_pc());
                    return mf;
                }else{
                    mf = NULL;
                }
            }    
        }
    }
    return mf;
}

void router::pop_reply(unsigned module_number, unsigned long cycle){
    bool first_buffer;
    this->first_buffer ? first_buffer = false : first_buffer = true;
    mem_fetch* mf = NULL;
    for (auto& par : buffers) {
        if(module_number == std::get<3>(par.first) &&
            !std::get<0>(par.first) && !std::get<1>(par.first)){
            if (!first_buffer){ first_buffer = true; continue;}
            if(!par.second->empty()){
                mf = par.second->top();
                if (mf->get_priority() <= cycle){
                    par.second->pop();
                    return;
                }
            }    
        }
    }
}

void router::pop_request(unsigned module_number, unsigned long cycle){
    bool first_buffer;
    this->first_buffer ? first_buffer = false : first_buffer = true;
    mem_fetch* mf = NULL;
    for (auto& par : buffers) {
        if(module_number == std::get<3>(par.first) &&
            std::get<0>(par.first) && !std::get<1>(par.first)){
            if (!first_buffer){ first_buffer = true; continue;}
            if(!par.second->empty()){
                mf = par.second->top();
                if (mf->get_priority() <= cycle){
                    par.second->pop();
                    return;
                }
            }    
        }
    }
}

void router::cycle(unsigned long cycle){
    for (auto& par : buffers) {
        if(std::get<1>(par.first) && par.second){
            par.second->cycle(true,cycle);
        }else if(par.second){
            par.second->cycle(false,cycle);
        }
    }
}