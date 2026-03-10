// Copyright (c) 2009-2011, Tor M. Aamodt, Wilson W.L. Fung, Ali Bakhoda
// The University of British Columbia
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution. Neither the name of
// The University of British Columbia nor the names of its contributors may be
// used to endorse or promote products derived from this software without
// specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include "icnt_wrapper.h"
#include <assert.h>
#include "../intersim2/globals.hpp"
#include "../intersim2/interconnect_interface.hpp"
#include "local_interconnect.h"

icnt_create_p* icnt_create;
icnt_init_p* icnt_init;
icnt_has_buffer_p* icnt_has_buffer;
icnt_push_p* icnt_push;
icnt_pop_p* icnt_pop;
icnt_transfer_p* icnt_transfer;
icnt_busy_p* icnt_busy;
icnt_display_stats_p* icnt_display_stats;
icnt_display_overall_stats_p* icnt_display_overall_stats;
icnt_display_state_p* icnt_display_state;
icnt_get_flit_size_p* icnt_get_flit_size;

unsigned g_network_mode;
char* g_network_config_filename;

struct inct_config g_inct_config;
LocalInterconnect** g_localicnt_interface;

#include "../option_parser.h"

// Wrapper to intersim2 to accompany old icnt_wrapper
// TODO: use delegate/boost/c++11<funtion> instead

static void intersim2_create(unsigned int n_shader, unsigned int n_mem, unsigned int network) {
  g_icnt_interface[network]->CreateInterconnect(n_shader, n_mem);
}

static void intersim2_init(unsigned int network) { g_icnt_interface[network]->Init(); }

static bool intersim2_has_buffer(unsigned input, unsigned int size, unsigned int network) {
  return g_icnt_interface[network]->HasBuffer(input, size);
}

static void intersim2_push(unsigned input, unsigned output, void* data,
                           unsigned int size, unsigned int network) {
  g_icnt_interface[network]->Push(input, output, data, size);
}

static void* intersim2_pop(unsigned output,unsigned int network) {
  return g_icnt_interface[network]->Pop(output);
}

static void intersim2_transfer(unsigned int network) { g_icnt_interface[network]->Advance(); }

static bool intersim2_busy(unsigned int network) { return g_icnt_interface[network]->Busy(); }

static void intersim2_display_stats(unsigned int network) { g_icnt_interface[network]->DisplayStats(); }

static void intersim2_display_overall_stats(unsigned int network) {
  g_icnt_interface[network]->DisplayOverallStats();
}

static void intersim2_display_state(FILE* fp, unsigned int network) {
  g_icnt_interface[network]->DisplayState(fp);
}

static unsigned intersim2_get_flit_size(unsigned int network) {
  return g_icnt_interface[network]->GetFlitSize();
}

//////////////////////////////////////////////////////

static void LocalInterconnect_create(unsigned int n_shader,
                                     unsigned int n_mem, unsigned int network) {
  g_localicnt_interface[network]->CreateInterconnect(n_shader, n_mem);
}

static void LocalInterconnect_init(unsigned int network) { g_localicnt_interface[network]->Init(); }

static bool LocalInterconnect_has_buffer(unsigned input, unsigned int size, unsigned int network) {
  return g_localicnt_interface[network]->HasBuffer(input, size);
}

static void LocalInterconnect_push(unsigned input, unsigned output, void* data,
                                   unsigned int size, unsigned int network) {
  g_localicnt_interface[network]->Push(input, output, data, size);
}

static void* LocalInterconnect_pop(unsigned output, unsigned int network) {
  return g_localicnt_interface[network]->Pop(output);
}

static void LocalInterconnect_transfer(unsigned int network) { g_localicnt_interface[network]->Advance(); }

static bool LocalInterconnect_busy(unsigned int network) { return g_localicnt_interface[network]->Busy(); }

static void LocalInterconnect_display_stats(unsigned int network) {
  g_localicnt_interface[network]->DisplayStats();
}

static void LocalInterconnect_display_overall_stats(unsigned int network) {
  g_localicnt_interface[network]->DisplayOverallStats();
}

static void LocalInterconnect_display_state(FILE* fp, unsigned int network) {
  g_localicnt_interface[network]->DisplayState(fp);
}

static unsigned LocalInterconnect_get_flit_size(unsigned int network) {
  return g_localicnt_interface[network]->GetFlitSize();
}

///////////////////////////

void icnt_reg_options(class OptionParser* opp) {
  option_parser_register(opp, "-network_mode", OPT_INT32, &g_network_mode,
                         "Interconnection network mode", "1");
  option_parser_register(opp, "-inter_config_file", OPT_CSTR,
                         &g_network_config_filename,
                         "Interconnection network config file", "mesh");

  // parameters for local xbar
  option_parser_register(opp, "-icnt_in_buffer_limit", OPT_UINT32,
                         &g_inct_config.in_buffer_limit, "in_buffer_limit",
                         "64");
  option_parser_register(opp, "-icnt_out_buffer_limit", OPT_UINT32,
                         &g_inct_config.out_buffer_limit, "out_buffer_limit",
                         "64");
  option_parser_register(opp, "-icnt_subnets", OPT_UINT32,
                         &g_inct_config.subnets, "subnets", "2");
  option_parser_register(opp, "-icnt_arbiter_algo", OPT_UINT32,
                         &g_inct_config.arbiter_algo, "arbiter_algo", "1");
  option_parser_register(opp, "-icnt_verbose", OPT_UINT32,
                         &g_inct_config.verbose, "inct_verbose", "0");
  option_parser_register(opp, "-icnt_grant_cycles", OPT_UINT32,
                         &g_inct_config.grant_cycles, "grant_cycles", "1");
}

void icnt_wrapper_init(int number_of_networks, double icnt_freq) {
  switch (g_network_mode) {
    case INTERSIM:
      // FIXME: delete the object: may add icnt_done wrapper
      g_icnt_interface = static_cast<InterconnectInterface**>(std::malloc(number_of_networks * sizeof(InterconnectInterface*)));
      icnt_create = static_cast<icnt_create_p*>(std::malloc(number_of_networks * sizeof(icnt_create_p)));
      icnt_init = static_cast<icnt_init_p*>(std::malloc(number_of_networks * sizeof(icnt_init_p)));
      icnt_has_buffer = static_cast<icnt_has_buffer_p*>(std::malloc(number_of_networks * sizeof(icnt_has_buffer_p)));
      icnt_push = static_cast<icnt_push_p*>(std::malloc(number_of_networks * sizeof(icnt_push_p)));
      icnt_pop = static_cast<icnt_pop_p*>(std::malloc(number_of_networks * sizeof(icnt_pop_p)));
      icnt_transfer = static_cast<icnt_transfer_p*>(std::malloc(number_of_networks * sizeof(icnt_transfer_p)));
      icnt_busy = static_cast<icnt_busy_p*>(std::malloc(number_of_networks * sizeof(icnt_busy_p)));
      icnt_display_stats = static_cast<icnt_display_stats_p*>(std::malloc(number_of_networks * sizeof(icnt_display_stats_p)));
      icnt_display_overall_stats = static_cast<icnt_display_overall_stats_p*>(std::malloc(number_of_networks * sizeof(icnt_display_overall_stats_p)));
      icnt_display_state = static_cast<icnt_display_state_p*>(std::malloc(number_of_networks * sizeof(icnt_display_state_p)));
      icnt_get_flit_size = static_cast<icnt_get_flit_size_p*>(std::malloc(number_of_networks * sizeof(icnt_get_flit_size_p)));
      for (int i = 0; i < number_of_networks; i++){
        g_icnt_interface[i] = InterconnectInterface::New(g_network_config_filename, icnt_freq);
        icnt_create[i] = intersim2_create;
        icnt_init[i] = intersim2_init;
        icnt_has_buffer[i] = intersim2_has_buffer;
        icnt_push[i] = intersim2_push;
        icnt_pop[i] = intersim2_pop;
        icnt_transfer[i] = intersim2_transfer;
        icnt_busy[i] = intersim2_busy;
        icnt_display_stats[i] = intersim2_display_stats;
        icnt_display_overall_stats[i] = intersim2_display_overall_stats;
        icnt_display_state[i] = intersim2_display_state;
        icnt_get_flit_size[i] = intersim2_get_flit_size;
      }
      break;
    case LOCAL_XBAR:
      g_localicnt_interface = static_cast<LocalInterconnect**>(std::malloc(number_of_networks * sizeof(LocalInterconnect*)));
      icnt_create = static_cast<icnt_create_p*>(std::malloc(number_of_networks * sizeof(icnt_create_p)));
      icnt_init = static_cast<icnt_init_p*>(std::malloc(number_of_networks * sizeof(icnt_init_p)));
      icnt_has_buffer = static_cast<icnt_has_buffer_p*>(std::malloc(number_of_networks * sizeof(icnt_has_buffer_p)));
      icnt_push = static_cast<icnt_push_p*>(std::malloc(number_of_networks * sizeof(icnt_push_p)));
      icnt_pop = static_cast<icnt_pop_p*>(std::malloc(number_of_networks * sizeof(icnt_pop_p)));
      icnt_transfer = static_cast<icnt_transfer_p*>(std::malloc(number_of_networks * sizeof(icnt_transfer_p)));
      icnt_busy = static_cast<icnt_busy_p*>(std::malloc(number_of_networks * sizeof(icnt_busy_p)));
      icnt_display_stats = static_cast<icnt_display_stats_p*>(std::malloc(number_of_networks * sizeof(icnt_display_stats_p)));
      icnt_display_overall_stats = static_cast<icnt_display_overall_stats_p*>(std::malloc(number_of_networks * sizeof(icnt_display_overall_stats_p)));
      icnt_display_state = static_cast<icnt_display_state_p*>(std::malloc(number_of_networks * sizeof(icnt_display_state_p)));
      icnt_get_flit_size = static_cast<icnt_get_flit_size_p*>(std::malloc(number_of_networks * sizeof(icnt_get_flit_size_p)));
      
      for (int i = 0; i < number_of_networks; i++){
        g_localicnt_interface[i] = LocalInterconnect::New(g_inct_config);
        icnt_create[i] = LocalInterconnect_create;
        icnt_init[i] = LocalInterconnect_init;
        icnt_has_buffer[i] = LocalInterconnect_has_buffer;
        icnt_push[i] = LocalInterconnect_push;
        icnt_pop[i] = LocalInterconnect_pop;
        icnt_transfer[i] = LocalInterconnect_transfer;
        icnt_busy[i] = LocalInterconnect_busy;
        icnt_display_stats[i] = LocalInterconnect_display_stats;
        icnt_display_overall_stats[i] = LocalInterconnect_display_overall_stats;
        icnt_display_state[i] = LocalInterconnect_display_state;
        icnt_get_flit_size[i] = LocalInterconnect_get_flit_size;
      }
      break;
    default:
      assert(0);
      break;
  }
}
