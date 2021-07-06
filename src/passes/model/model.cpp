/*
 * -----------------------------------------------------------------
 * COMPANY : Ruhr-Universität Bochum, Chair for Security Engineering
 * AUTHOR  : Jan Richter-Brockmann (jan.richter-brockmann@rub.de)
 *           Pascal Sasdrich (pascal.sasdrich@rub.de)
 * DOCUMENT: ---
 * -----------------------------------------------------------------
 *
 * Copyright (c) 2020, Jan Richter-Brockmann, Pascal Sasdrich
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

#include "passes/model/model.hpp"

#include <iostream>
#include <map>

#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/topological_sort.hpp"

#include "util.hpp"


int verify::passes::model::get_blacklisted_entities(const std::string file, std::vector<std::string> &blacklistedEntities){
    std::string line;
    std::vector<std::string> tokens;
    
    // Open blacklist file.
    std::ifstream description(file);

    // Loop over all lines and all names per line
    while(std::getline(description, line)){
        // Split line into separate names
        tokens = split(line, ' ');

        // push entity names to vector in case it is not a comment
        if(tokens[0].at(0) != '#'){
            for(auto t=tokens.begin(); t!=tokens.end(); ++t){
                blacklistedEntities.push_back(*t);
            }
        }
    }

    // Close circuit description text file.
    description.close();

    return 0;
}


int verify::passes::model::get_blacklisted_nodes(fiver::Circuit golden, std::vector<fiver::Gate> &blacklist){
    // Find all gates that are labeled as blacklisted
    for (auto gate = vertices(golden).first; gate != vertices(golden).second; ++gate) {
        if(golden[*gate].blacklisted) blacklist.push_back(*gate);
    }

    return 0;
}


int verify::passes::model::get_fault_type(const std::string file, std::map<fiver::GateType, std::vector<verify::passes::analysis::Fault>> &faultMap){
    // Variable declarations
    std::vector<std::string> tokens;
    std::string line, token;

    // Open circuit description text file.
    std::ifstream description(file);

    // Parse description line by line
    while(std::getline(description, line)){
        // Split line in tokens
        tokens = split(line, ' ');

        fiver::GateType identifier;

        if(tokens[0].at(0) != '#'){
            if(tokens[0] == "in"){
                identifier = fiver::GateType::IN;
            } else if (tokens[0] == "out"){
                identifier = fiver::GateType::OUT;
            } else if (tokens[0] == "reg"){
                identifier = fiver::GateType::REG;
            } else if (tokens[0] == "buf"){
                identifier = fiver::GateType::BUF;
            } else if (tokens[0] == "not"){
                identifier = fiver::GateType::NOT;
            } else if (tokens[0] == "and"){
                identifier = fiver::GateType::AND;
            } else if (tokens[0] == "nand"){
                identifier = fiver::GateType::NAND;
            } else if (tokens[0] == "or"){
                identifier = fiver::GateType::OR;
            } else if (tokens[0] == "nor"){
                identifier = fiver::GateType::NOR;
            } else if (tokens[0] == "xor"){
                identifier = fiver::GateType::XOR;
            } else if (tokens[0] == "xnor"){
                identifier = fiver::GateType::XNOR;
            } else {
                std::cout << "[ERR-FAULT-MODEL] Input gate type - " << tokens[0] << " - is not supported for fault map!" << std::endl;
            }
        }

        // Fill fault map
        if(tokens[0].at(0) != '#'){
            for(int it=1; it < tokens.size(); it++){
                if(tokens[it] == "and"){
                    faultMap[identifier].push_back(verify::passes::analysis::Fault::AND);
                }
                else if(tokens[it] == "nand"){
                    faultMap[identifier].push_back(verify::passes::analysis::Fault::NAND);
                }
                else if(tokens[it] == "or"){
                    faultMap[identifier].push_back(verify::passes::analysis::Fault::OR);
                }
                else if(tokens[it] == "nor"){
                    faultMap[identifier].push_back(verify::passes::analysis::Fault::NOR);
                }
                else if(tokens[it] == "xor"){
                    faultMap[identifier].push_back(verify::passes::analysis::Fault::XOR);
                }
                else if(tokens[it] == "xnor"){
                    faultMap[identifier].push_back(verify::passes::analysis::Fault::XNOR);
                }
                else if(tokens[it] == "reset"){
                    faultMap[identifier].push_back(verify::passes::analysis::Fault::RESET);
                }
                else if(tokens[it] == "set"){
                    faultMap[identifier].push_back(verify::passes::analysis::Fault::SET);
                }
                else if(tokens[it] == "buf"){
                    faultMap[identifier].push_back(verify::passes::analysis::Fault::A);
                }
                else if(tokens[it] == "not"){
                    faultMap[identifier].push_back(verify::passes::analysis::Fault::NOTA);
                } else {
                    std::cout << "[ERR-FAULT-MODEL] Fault type for fault mapping is not supported! " << tokens[0] << " " << tokens[it] << std::endl;
                }
            }
        }     
    }

    // Close circuit description text file.
    description.close();

    return 0;
}


int verify::passes::model::get_fault_location(const std::string location, fiver::Circuit golden, std::vector<fiver::Gate> &faultLocations, std::vector<fiver::Gate> &blacklist) {
    if(location == "s" || location == "cs"){
        for (auto gate = vertices(golden).first; gate != vertices(golden).second; ++gate) {
            // push each register to the fault location list
            if (golden[*gate].type == fiver::GateType::REG) {
                if(std::find(blacklist.begin(), blacklist.end(), *gate) == blacklist.end()){
                    faultLocations.push_back(*gate);
                }
            }
        }

    } 
    
    if (location == "c" || location == "cs"){
        for (auto gate = vertices(golden).first; gate != vertices(golden).second; ++gate) {
            // push each combinational gate to the fault location list
            if(std::find(blacklist.begin(), blacklist.end(), *gate) == blacklist.end()){
                if (golden[*gate].type != fiver::GateType::IN && golden[*gate].type != fiver::GateType::OUT && golden[*gate].type != fiver::GateType::REG) {
                    faultLocations.push_back(*gate);
                }
            }
        }

    }

    if (location != "c" && location != "s" && location != "cs"){
        std::cout << "[ERR-FAULT-MODEL] Fault location is not supported!" << std::endl;
    }

    return 0;
}



// Naive approach, just adds all registers, input gates to registers and spreading gates to the reduced list (generates largest set of reducedFaultLocations)
int verify::passes::model::get_reduced_fault_location_naive(fiver::Circuit &golden, std::vector<fiver::Gate> &faultLocations, std::vector<fiver::Gate> &reducedFaultLocations){
    std::map<fiver::Gate, std::vector<fiver::Gate>> infected_gates;

    // First, add registers and identify infected gates
    for(auto &gate : faultLocations){
        if(golden[gate].type == fiver::GateType::REG){
            reducedFaultLocations.push_back(gate);
        }
        else {
            // Loop over propagation path
            int reg_cnt = 0;
            for(auto &prop : golden[gate].propagation_path){
                if(golden[gate].stage_index == golden[prop].stage_index && (golden[prop].type == fiver::GateType::REG || golden[prop].type == fiver::GateType::OUT)){
                    ++reg_cnt;
                    infected_gates[gate].push_back(prop);
                }
            }
        }
    }

    // Second, looking for registers and output gates - the input nodes are added to the reducedFaultLocation vector
    std::vector<fiver::Gate> sorted; boost::topological_sort(golden, std::back_inserter(sorted));
    typename boost::graph_traits<fiver::Circuit>::in_edge_iterator n_start, n_end, next;

    for(auto &g : sorted){
        if(golden[g].type == fiver::GateType::REG || golden[g].type == fiver::GateType::OUT){
            boost::tie(n_start, n_end) = in_edges(g, golden);

            if(std::find(faultLocations.begin(), faultLocations.end(), source(*n_start, golden)) != faultLocations.end()) reducedFaultLocations.push_back(source(*n_start, golden)); 
        }
    }

    // Third, add spreading gates
    // Goining backwards drastically increase the performence in the evaluation phase
    std::vector<std::vector<fiver::Gate>> done_vectors;
    for(auto gate=infected_gates.rbegin(); gate != infected_gates.rend(); ++gate){
        if(gate->second.size() > 1){
            if(std::find(reducedFaultLocations.begin(), reducedFaultLocations.end(), gate->first) == reducedFaultLocations.end()) reducedFaultLocations.push_back(gate->first);
        }
    }

    return 0;
}


// Aggressive method creating the smallest set of reducedFaultLocations
int verify::passes::model::get_reduced_fault_location_aggrassive(fiver::Circuit &golden, std::vector<fiver::Gate> &faultLocations, std::vector<fiver::Gate> &reducedFaultLocations){
    std::map<fiver::Gate, std::vector<fiver::Gate>> infected_gates;

    // First, add registers and identify infected gates
    for(auto &gate : faultLocations){
        if(golden[gate].type == fiver::GateType::REG){
            reducedFaultLocations.push_back(gate);
        }
        else {
            // Loop over propagation path
            int reg_cnt = 0;
            for(auto &prop : golden[gate].propagation_path){
                if(golden[gate].stage_index == golden[prop].stage_index && (golden[prop].type == fiver::GateType::REG || golden[prop].type == fiver::GateType::OUT)){
                    ++reg_cnt;
                    infected_gates[gate].push_back(prop);
                }
            }
        }
    }

    // Second, looking for registers and output gates - the input nodes are added to the reducedFaultLocation vector
    std::vector<fiver::Gate> sorted; boost::topological_sort(golden, std::back_inserter(sorted));
    typename boost::graph_traits<fiver::Circuit>::in_edge_iterator n_start, n_end, next;

    for(auto &g : sorted){
        if(golden[g].type == fiver::GateType::REG || golden[g].type == fiver::GateType::OUT){
            boost::tie(n_start, n_end) = in_edges(g, golden);

            if(std::find(faultLocations.begin(), faultLocations.end(), source(*n_start, golden)) != faultLocations.end()) reducedFaultLocations.push_back(source(*n_start, golden)); 
        }
    }

    // for(auto const& [key, val] : infected_gates){
    //     if(val.size() > 1){
    //         std::cout << "Key " << key << ": ";
    //         for(auto& print_vec : val){
    //             std::cout << print_vec << " ";
    //         }
    //         std::cout << std::endl;
    //     }
    // }
    
    // Third, unify infected_gates
    // Goining backwards drastically increase the performence in the evaluation phase
    std::vector<std::vector<fiver::Gate>> done_vectors;
    for(auto gate=infected_gates.rbegin(); gate != infected_gates.rend(); ++gate){
        if(gate->second.size() > 1){
            bool isIn = false;
            for(auto &done : done_vectors){
                if(done == gate->second){
                    isIn = true;
                }
            }
            if(!isIn){
                done_vectors.push_back(gate->second);
                reducedFaultLocations.push_back(gate->first);
            }

        }
    }

    return 0;
}



// Complexity reduction
// Applies backwards search
int verify::passes::model::get_reduced_fault_location_conservative(fiver::Circuit &golden, std::vector<fiver::Gate> &faultLocations, std::vector<fiver::Gate> &reducedFaultLocations){
    typename boost::graph_traits<fiver::Circuit>::in_edge_iterator n_start, n_end, next;
    std::vector<fiver::Gate> sorted; boost::topological_sort(golden, std::back_inserter(sorted));

    // First, identify all registers and outputs
    // push these gates already to reducedFaultLocations if it is inlcuded in faultLocations
    std::vector<fiver::Gate> startingGates;
    for(auto &g : sorted){
        if(golden[g].type == fiver::GateType::REG || golden[g].type == fiver::GateType::OUT){
            startingGates.push_back(g);          // fill startingGates
            if(std::find(faultLocations.begin(), faultLocations.end(), g) != faultLocations.end()) reducedFaultLocations.push_back(g);  // fill reducedFaultLocations
        }
    }

    //std::cout << "Length starting Gates: " << startingGates.size() << std::endl;

    // Second, search backwards from all startingGates (only in current logic stage)
    // Identify all spreading gates 
    for(auto &start : startingGates){
        // input to start gate is always added to reducedFaultLocations
        boost::tie(n_start, n_end) = in_edges(start, golden);
        if(std::find(faultLocations.begin(), faultLocations.end(), source(*n_start, golden)) != faultLocations.end()) reducedFaultLocations.push_back(source(*n_start, golden));

        // search backwards and idendify spreading gates
        std::vector<fiver::Gate> searchList;
        searchList.push_back(source(*n_start, golden));
        while(!searchList.empty()){
            fiver::Gate elem = searchList.front();
            searchList.erase(searchList.begin());

            // Find all in-nodes and add them to searchList
            boost::tie(n_start, n_end) = in_edges(elem, golden);
            for(auto next=n_start; next!=n_end; ++next){
                if(golden[source(*next, golden)].type != fiver::GateType::REG && golden[source(*next, golden)].type != fiver::GateType::IN){
                    searchList.push_back(source(*next, golden));
                }
            }

            // Add spreading gate to reducedFaultLocations if it is included in faultLocations (i.e., it is a valid fault location)
            if(out_degree(elem, golden) > 1){
                if(std::find(faultLocations.begin(), faultLocations.end(), elem) != faultLocations.end()){
                    if(std::find(reducedFaultLocations.begin(), reducedFaultLocations.end(), elem) == reducedFaultLocations.end()) reducedFaultLocations.push_back(elem);
                } 
            } 
        }
    }

    return 0;
}