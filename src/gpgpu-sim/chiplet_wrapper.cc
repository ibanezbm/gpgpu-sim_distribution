#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <tuple>
#include <queue>
#include <unordered_map>
#include "mem_fetch.h"
#include "../intersim2/chiplet_interface.hpp"
#include "../option_parser.h"
#include "ring_connection.h"

unsigned g_chiplet_mode;
char* g_chiplet_config_filename;

void chiplet_reg_options(class OptionParser* opp) {
  option_parser_register(opp, "-chiplet_mode", OPT_INT32, &g_chiplet_mode,
                         "Interconnection chiplet mode", "2");

  option_parser_register(opp, "-inter_chiplet_config_file", OPT_CSTR,
                         &g_chiplet_config_filename,
                         "Interconnection chiplet config file", "mesh");

}


ChipletInterconnection* chiplet_wrapper_init(int number_of_nodes, double chiplet_freq) {
    // Initialize chiplet interconnection based on g_chiplet_mode and g_chiplet_config_filename
    ChipletInterconnection* chiplet_icnt = nullptr;
    switch (g_chiplet_mode) {
        case CHIPLET_INTERSIM:
            // Initialize intersim chiplet interconnection
            chiplet_icnt = new ChipletInterface(number_of_nodes, g_chiplet_config_filename);
            //break;
        case CHIPLET_RING:
            // Initialize local ring chiplet interconnection
            chiplet_icnt = new ring(number_of_nodes, chiplet_freq); // Assuming frequency is 1.0 GHz
            break;
        default:
            fprintf(stderr, "Unknown chiplet interconnection mode: %d\n", g_chiplet_mode);
            exit(EXIT_FAILURE);
    }
    return chiplet_icnt;
}