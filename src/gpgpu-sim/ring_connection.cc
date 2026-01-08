#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mem_fetch.h"
#include "ring_connection.h"
#include <cstdlib> 
#include <ctime>   
#include <queue>
#include <cstddef>


ring::ring(unsigned number_of_networks, double freq){
    number_of_chiplets = number_of_networks;
    unsigned links_per_chiplet = 4;
    link_name::link* links = (link_name::link*)malloc(sizeof(link_name::link)*number_of_networks*links_per_chiplet);
    buffer* buffers = (buffer*)malloc(sizeof(buffer)*number_of_networks*links_per_chiplet*2);
    
    for(unsigned i = 0; i < number_of_networks; i++){
        routers[i] = new router(number_of_networks, freq);
        moved_bytes_per_chiplet.push_back(0);
        messages_per_chiplet.push_back(0);
        latency_L2_to_RAM.push_back(0);
        latency_RAM_to_L2.push_back(0);
        latency_L2_to_ICNT.push_back(0);
        messages_per_chiplet_stats.push_back(0);
    }

    for(unsigned j = 0; j < links_per_chiplet*number_of_networks; j++){
        links[j] = *(new link_name::link());
        buffers[j*2] = *(new buffer(&links[j]));
        buffers[j*2+1] = *(new buffer(&links[j]));
    }

    for(unsigned i = 0; i < number_of_networks; i++){
        unsigned next = (i+1)%number_of_networks;
        for(unsigned j = 0; j < links_per_chiplet; j++){
            routers.at(i)->add_buffer(j%2, j<2,j<2? i: next, j<2? next: i, &(buffers[i*links_per_chiplet*2+j*2]));
            routers.at(next)->add_buffer(j%2, j>=2,j<2? i: next, j<2? next: i, &(buffers[i*links_per_chiplet*2+j*2+1]));
        }
    }
}

void ring::push_reply(unsigned input, unsigned output, mem_fetch* mf, unsigned int size, unsigned long cycle){
    if(mf->m_status_L2_to_RAM < 5000 && mf->m_status_L2_to_RAM != 0 &&
        mf->m_status_RAM_to_L2 < 5000 && mf->m_status_RAM_to_L2 != 0 &&
        mf->m_status_L2_to_ICNT < 5000 && mf->m_status_L2_to_ICNT != 0 ){

        latency_L2_to_RAM[mf->get_chiplet()] += mf->m_status_L2_to_RAM;
        latency_RAM_to_L2[mf->get_chiplet()] += mf->m_status_RAM_to_L2;
        latency_L2_to_ICNT[mf->get_chiplet()] += mf->m_status_L2_to_ICNT;
        messages_per_chiplet_stats[mf->get_chiplet()] += 1;
    }
    moved_bytes_per_chiplet[mf->get_chiplet()] += size;
    //printf("MF reply uid:%d warp:%d tpc:%d, pc:%d chip:%d type:%d\n", mf->get_request_uid(), mf->get_wid(), mf->get_tpc(), mf->get_pc(), mf->get_chiplet(),mf->get_access_type());
    messages_per_chiplet[mf->get_chiplet()] += 1;
    routers[input]->push_reply(input, output, mf, size, cycle);
}

void ring::push_request(unsigned input, unsigned output, mem_fetch* mf, unsigned int size, unsigned long cycle){
    /*std::srand(static_cast<unsigned int>(0));
    int random_number = (std::rand() % 2) + 1;
    if((input == 0 && output == 3)){
        output = random_number;
    }else if(((input == 3 && output == 0))){
        output = random_number;
    }else if((input == 1 && output == 2)){
        output = (random_number-1)*3;
    }else if((input == 2 && output == 1)){
        output = (random_number-1)*3;
    }
    mf->set_priority(cycle+32);
    buffers_request[std::make_pair(input,output)].push_back(mf);*/
    moved_bytes_per_chiplet[mf->get_chiplet()] += size;
    //printf("MF request uid:%d warp:%d tpc:%d, pc:%d chip:%d type:%d\n", mf->get_request_uid(), mf->get_wid(), mf->get_tpc(), mf->get_pc(), mf->get_chiplet(), mf->get_access_type());
    messages_per_chiplet[mf->get_chiplet()] += 1;
    routers[input]->push_request(input, output, mf, size, cycle);
}

bool ring::has_buffer_reply(unsigned input, unsigned output, unsigned int size){
    return routers[input]->has_buffer_reply(input, output, size);
}
bool ring::has_buffer_request(unsigned input, unsigned output, unsigned int size){
    return routers[input]->has_buffer_request(input, output, size);
}

mem_fetch* ring::top_reply(unsigned module_number, unsigned long cycle, bool first_buffer){
    /*
    mem_fetch* mf = NULL;
    for (auto& par : ring_connections_reply) {
        if(module_number == std::get<1>(par.first)){
            if (!first_buffer){ first_buffer = true; continue;}
            if(!par.second.empty()){
                mf = par.second.top();
                if (mf->get_priority() <= cycle){
                    return mf;
                }else{
                    mf = NULL;
                }
            }    
        }
    }*/
    return routers[module_number]->top_reply(module_number, cycle,first_buffer);
    //return mf;
}

void ring::pop_reply(unsigned module_number, unsigned long cycle, bool first_buffer){
    /*
    mem_fetch* mf = NULL;
    for (auto& par : ring_connections_reply) {
        if(module_number == std::get<1>(par.first)){
            if (!first_buffer){ first_buffer = true; continue;}
            if(!par.second.empty()){
                mf = par.second.top();
                if (mf->get_priority() <= cycle){
                    par.second.pop();
                    return;
                }
            }    
        }
    }*/
    routers[module_number]->pop_reply(module_number, cycle,first_buffer);
}

mem_fetch* ring::top_request(unsigned module_number,unsigned long cycle, bool first_buffer){
    /*
    mem_fetch* mf = NULL;
    for (auto& par : ring_connections_request) {
        if(module_number == std::get<1>(par.first)){
            if (!first_buffer){ first_buffer = true; continue;}
            if(!par.second.empty()){
                mf = par.second.top();
                if (mf->get_priority() <= cycle){
                    return mf;
                }else{
                    mf = NULL;
                }
            }
        }
    }*/
    return routers[module_number]->top_request(module_number, cycle,first_buffer);
    //return mf;  
}

void ring::pop_request(unsigned module_number, unsigned long cycle, bool first_buffer){
    /*
    mem_fetch* mf = NULL;
    for (auto& par : ring_connections_request) {
        if(module_number == std::get<1>(par.first)){
            if (!first_buffer){ first_buffer = true; continue;}
            if(!par.second.empty()){
                mf = par.second.top();
                if (mf->get_priority() <= cycle){
                    par.second.pop();
                    return;
                } 
            }    
        }
    }*/
    routers[module_number]->pop_request(module_number, cycle,first_buffer);
}

void ring::cycle(unsigned long cycle){
    /*
    for(unsigned i = 0; i < 4; i++){
        for(unsigned j = 0; j < 4; j++){
            if(buffers_reply.find(std::make_pair(i,j)) != buffers_reply.end() || 
               buffers_request.find(std::make_pair(i,j)) != buffers_request.end()){
                if(!buffers_reply[std::make_pair(i,j)].empty()){
                    mem_fetch* mf = buffers_reply[std::make_pair(i,j)].front();
                    buffers_reply[std::make_pair(i,j)].pop_front();
                    ring_connections_reply[std::make_pair(i,j)].push(mf);
                }
                if(!buffers_request[std::make_pair(i,j)].empty()){
                    mem_fetch* mf = buffers_request[std::make_pair(i,j)].front();
                    buffers_request[std::make_pair(i,j)].pop_front();
                    ring_connections_request[std::make_pair(i,j)].push(mf);
                }
            }
        }    
    }*/
    for(unsigned i = 0; i<number_of_chiplets; i++){
        routers[i]->cycle(cycle);
    }
    cycles++;
}

void ring::print_stats(){
    for (size_t i = 0; i < moved_bytes_per_chiplet.size(); ++i) {
        printf("moved_bytes_per_chiplet[%ld] = %lld\n", i, moved_bytes_per_chiplet[i]);
    }
    for (size_t i = 0; i < messages_per_chiplet.size(); ++i) {
        printf("messages_per_chiplet[%ld] = %lld\n", i, messages_per_chiplet[i]);
    }
    for (size_t i = 0; i < latency_L2_to_RAM.size(); ++i) {
        if(messages_per_chiplet_stats[i]==0){
            printf("latency_L2_to_RAM[%ld] = 0\n",i);
        }else{
            printf("latency_L2_to_RAM[%ld] = %f\n", i, float(latency_L2_to_RAM[i])/messages_per_chiplet_stats[i]);
        }
    }
    for (size_t i = 0; i < latency_RAM_to_L2.size(); ++i) {
        if(messages_per_chiplet_stats[i]==0){
            printf("latency_RAM_to_L2[%ld] = 0\n",i);
        }else{
            printf("latency_RAM_to_L2[%ld] = %f\n", i, float(latency_RAM_to_L2[i])/messages_per_chiplet_stats[i]);
        }
    }
    for (size_t i = 0; i < latency_L2_to_ICNT.size(); ++i) {
        if(messages_per_chiplet_stats[i]==0){
            printf("latency_L2_to_ICNT[%ld] = 0\n",i);
        }else{
            printf("latency_L2_to_ICNT[%ld] = %f\n", i, float(latency_L2_to_ICNT[i])/messages_per_chiplet_stats[i]);
        }
    }
    for (size_t i = 0; i < messages_per_chiplet_stats.size(); ++i) {
        printf("messages_per_chiplet_stats[%ld] = %lld\n", i, messages_per_chiplet_stats[i]);
    }
    for (size_t i = 0; i < latency_L2_to_RAM.size(); ++i) {
        printf("latency_L2_to_RAM[%ld] = %lld\n", i, latency_L2_to_RAM[i]);
    }
    for (size_t i = 0; i < latency_L2_to_RAM.size(); ++i) {
        printf("latency_RAM_to_L2[%ld] = %lld\n", i, latency_RAM_to_L2[i]);
    }
    for (size_t i = 0; i < latency_L2_to_RAM.size(); ++i) {
        printf("latency_L2_to_ICNT[%ld] = %lld\n", i, latency_L2_to_ICNT[i]);
    }
}