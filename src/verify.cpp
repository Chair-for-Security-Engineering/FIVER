/*
 * -----------------------------------------------------------------
 * COMPANY : Ruhr-Universität Bochum, Chair for Security Engineering
 * AUTHOR  : Pascal Sasdrich (pascal.sasdrich@rub.de)
 *           Jan Richter-Brockmann (jan.richter-brockmann@rub.de)
 * DOCUMENT: https://eprint.iacr.org/2021/936.pdf
 * -----------------------------------------------------------------
 *
 * Copyright (c) 2021, Jan Richter-Brockmann and Pascal Sasdrich
 *
 * All rights reserved.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTERS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Please see license.rtf and README for license and further instructions.
 */

#include <chrono>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <numeric>

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "boost/graph/topological_sort.hpp"

#include "circuit.hpp"
#include "util.hpp"

#include "frontends/frontends.hpp"

#include "passes/passes.hpp"

#define str(i) std::to_string(i)

using namespace fiver;

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::milli;

int main (int argc, char * argv[]) {

    /* Variable declarations */
    std::string circuitPath, configPath;

    /* Command-line parsing of arguments */
    try {
        boost::program_options::positional_options_description pdesc;
        pdesc.add("file", -1);

        boost::program_options::options_description desc{"Options"};
        desc.add_options()
            ("help,h", "Help screen")
            ("config,c", boost::program_options::value<std::string>()->default_value("config/strategy"), "Textual description of verification strategy");

        boost::program_options::variables_map vm;
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
        boost::program_options::notify(vm);

        if(vm.count("help")) {
            std::cout << desc << std::endl;
            return 0;
        } 
        
        if (vm.count("config")) {
            configPath = vm["config"].as<std::string>();
        }

    } catch (const boost::program_options::error &ex) {
        std::cerr << ex.what() << std::endl;
    }

    /* Strategy file parsing */
    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini(configPath, pt);

    /* Start BDD session */
    int num_cores = pt.get<int>("general.cores");
    std::vector<Cudd> bddManager(num_cores);
    for(int b=0; b<num_cores; ++b){
        bddManager[b] = Cudd(0,0,1<<4,1<<24,pt.get<int>("general.memory")*1024*1024*1024ull);
        bddManager[b].AutodynEnable(CUDD_REORDER_SIFT);
    }    

    /* Extract Designs Under Test */
    std::vector<std::string> circuits;
    std::string duts = pt.get<std::string>("circuit.dut");
    boost::algorithm::split(circuits, duts, boost::algorithm::is_space());

    /* Analyze all circuits */
    for (int dut = 0; dut < circuits.size(); dut++) {

        /* Start time tracking */
        std::chrono::time_point<std::chrono::high_resolution_clock> t1, t2;
        std::chrono::time_point<std::chrono::high_resolution_clock> start = time;

        /* Current DUT */
        INFO("Netlist: " + circuits[dut] + "\n", start);
        std::vector<Circuit> model(num_cores);

        // Get blacklisted entity names
        std::vector<std::string> blacklistedEntities;
        verify::passes::model::get_blacklisted_entities(pt.get<std::string>("fault.blacklist"), blacklistedEntities);

        /* Parse circuit from text file */
        if (circuits[dut].substr(circuits[dut].find_last_of(".")+1) == "nl") {
            fiver::frontend::instructions::parse(circuits[dut], model[0], blacklistedEntities);
        } else if (circuits[dut].substr(circuits[dut].find_last_of(".")+1) == "v") {
            int res = parse_and_convert_wrapper(pt.get<std::string>("library.file"), pt.get<std::string>("library.name"), circuits[dut], pt.get<std::string>("circuit.module"), circuits[dut].substr(0, circuits[dut].find_last_of(".")) + ".nl");
            if (res) { std::cerr << "[ERR] Verilog design parsing failed." << std::endl; return res; }
            fiver::frontend::instructions::parse(circuits[dut].substr(0, circuits[dut].find_last_of(".")) + ".nl", model[0], blacklistedEntities);
        } else {
            std::cerr << "[ERR] Unrecognized file extension." << std::endl; return 1;
        }
        if (pt.get<int>("general.verbose") > 0) INFO("Parse: " + str(num_vertices(model[0])) + " gate(s) / " + str(num_edges(model[0]))  + " signal(s)\n", start);

        /* Elabotare circuit model */
        // Copy models
        for(int e=1; e<num_cores; ++e) model[e] = model[0];
        
        // Elaborate models
        #pragma omp parallel for schedule(dynamic) num_threads(num_cores)
        for(int e=0; e<num_cores; ++e){
            fiver::passes::elaboration::elaborate(model[e], bddManager[e]);
        }
        if (pt.get<int>("general.verbose") > 0) INFO("Elaborate: " + str(get_property(model[0], &GraphContext::register_stages)) + " register stage(s) and " + str(get_property(model[0], &GraphContext::stages)) + " logic stages. \n", start);

        /* Design Properties */
        unsigned int comb_gates = 0;
        unsigned int sequ_gates = 0;
        for (auto gate = vertices(model[0]).first; gate != vertices(model[0]).second; ++gate) {
            if (model[0][*gate].type == fiver::GateType::REG) 
                sequ_gates++;
            else {
                if(model[0][*gate].type != fiver::GateType::IN && model[0][*gate].type != fiver::GateType::OUT)
                    comb_gates++;
            }
        }
        if (pt.get<int>("general.verbose") > 0) INFO("Elaborate: " + str(comb_gates) + " combinational gate(s) and " + str(sequ_gates) + " sequential gate(s). \n", start);

        /* Get number of faults */
        int num_of_faults = pt.get<int>("fault.number");

        /* Get setting for reduced complexity */
        int reduced_complexity = pt.get<int>("fault.reduced_complexity");
        if(reduced_complexity >= 4){
            std::cout << "[ERROR] Invalid setting for complexity reduction!" << std::endl;
        }

        /* Get variate type */
        int variate = pt.get<int>("fault.variate");
        if(variate > get_property(model[0], &GraphContext::stages)){   // check that variate is not larger than the total number of available logic stages
            std::cout << "Selected variate is greater than number of stages - set variate to " << get_property(model[0], &GraphContext::stages) << "." << std::endl;
            variate = get_property(model[0], &GraphContext::stages);
        }
        
        /* Get mapping for fault types */
        std::map<fiver::GateType, std::vector<verify::passes::analysis::Fault>> faultMap;
        verify::passes::model::get_fault_type(pt.get<std::string>("fault.mapping"), faultMap);

        /* Get fault locations */
        // define blacklist of gates that should _not_ be faulted
        std::vector<Gate> blacklist;
        verify::passes::model::get_blacklisted_nodes(model[0], blacklist);

        // Determine fault locations
        std::vector<Gate> faultLocations;
        verify::passes::model::get_fault_location(pt.get<std::string>("fault.location"), model[0], faultLocations, blacklist);

        /* Perform validity checks */
        if(num_of_faults > faultLocations.size()){
            std::cerr << "[ERR-PARAM] Number of selected faults is larger than available gates!" << std::endl;
            return -1;
        }

        /* Get counteremasure type */
        bool countermeasure_type = pt.get<bool>("circuit.correction_based");

        /* Get interruption flag */
        bool interrupt = pt.get<bool>("fault.interrupt");




        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // VERIFICATION
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /* 
        *   To speed up the fault injections and corresponding BDD evaluations,
        *   the propagation dependencies are determined for each gate.
        * */
        for(int p=0; p<num_cores; ++p) verify::passes::analysis::determine_propagation_path(model[p]);

        // Reduced fault locations
        std::vector<Gate> reducedFaultLocations;
        switch(reduced_complexity){
            case 1:
                std::cout << "Complexity reduction: apply naive approach..." << std::endl; 
                verify::passes::model::get_reduced_fault_location_naive(model[0], faultLocations, reducedFaultLocations);
                break;
            case 2:
                std::cout << "Complexity reduction: apply aggrassive approach..." << std::endl;
                verify::passes::model::get_reduced_fault_location_aggrassive(model[0], faultLocations, reducedFaultLocations);
                break;
            case 3:
                std::cout << "Complexity reduction: apply conservative approach..." << std::endl;
                verify::passes::model::get_reduced_fault_location_conservative(model[0], faultLocations, reducedFaultLocations);
                break;
        }
        std::cout << std::endl;


        /*
        *   Starting the actual fault injection and verification.
        */
        std::cout << "Valid fault locations: " << faultLocations.size() << std::endl;
        std::cout << "Reduced fault locations: " << reducedFaultLocations.size() << std::endl;

        // we do not need the dynamic reordering for the fault injection
        // since we only use the resulting BDDs once
        for(int bddman=0; bddman < bddManager.size(); ++bddman) bddManager[bddman].AutodynDisable();

        for(int iter=0;iter<1;++iter){
            const auto startTime = high_resolution_clock::now();
            verify::passes::analysis::fault_verification(model, bddManager, num_of_faults, variate, faultMap, faultLocations, reducedFaultLocations, reduced_complexity, countermeasure_type, interrupt);
            const auto endTime = high_resolution_clock::now();

            std::cout << "Time: " << duration_cast<duration<double, milli>>(endTime - startTime).count() << " ms" << std::endl;
        }

        model.clear();
    }


    return 0;
}