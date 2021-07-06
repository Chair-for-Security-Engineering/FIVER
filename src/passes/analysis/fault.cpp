/*
 * -----------------------------------------------------------------
 * COMPANY : Ruhr-Universität Bochum, Chair for Security Engineering
 * AUTHOR  : Jan Richter-Brockmann (jan.richter-brockmann@rub.de)
 *           Pascal Sasdrich (pascal.sasdrich@rub.de)
 * DOCUMENT: ---
 * -----------------------------------------------------------------
 *
 * Copyright (c) 2020, Jan Richter-Brockmann and Pascal Sasdrich
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

#include "passes/analysis/fault.hpp"

#include <iostream>

#include "util.hpp"

#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/topological_sort.hpp"
#include <boost/math/special_functions/binomial.hpp>

#include <omp.h>



void verify::passes::analysis::determine_propagation_path(fiver::Circuit &model){
    std::vector<fiver::Gate> sorted; boost::topological_sort(model, std::back_inserter(sorted));

    // Define iterator for output edges
    typename boost::graph_traits<fiver::Circuit>::out_edge_iterator n_start, n_end, next;
    
    for (auto node = sorted.rend()-1; node != sorted.rbegin()-1; --node) {
        boost::tie(n_start, n_end) = out_edges(*node, model);
        for(next = n_start; next != n_end; ++next){
            // use this with vectors
            for(auto it=model[target(*next, model)].propagation_path.begin(); it != model[target(*next, model)].propagation_path.end(); ++it){
                if(!is_in(model[*node].propagation_path, (std::size_t) *it)){
                    model[*node].propagation_path.push_back(*it);
                }
            }
            if(!is_in(model[*node].propagation_path, (std::size_t) target(*next, model))){
                model[*node].propagation_path.push_back(target(*next, model));
            }
        }   
    }
}


void verify::passes::analysis::elaborate_node(fiver::Circuit &model, Cudd& bddManager, fiver::Gate node){
    // Operand gates
    fiver::Gate op1, op2;

    // Read operand function(s) from source nodes(s)
    if (unary.find(model[node].type) != unary.end()) {
        op1 = source(*(in_edges(node, model).first+0), model);
    } else if (binary.find(model[node].type) != binary.end()) {
        op1 = source(*(in_edges(node, model).first+0), model);
        op2 = source(*(in_edges(node, model).first+1), model);
    }

    // Elaborate node
    switch (model[node].type){
        case fiver::GateType::OUT:
            model[node].function =  model[op1].function;
            break;
        case fiver::GateType::REG:
            model[node].function =  model[op1].function;
            break;
        case fiver::GateType::SET:
            model[node].function = bddManager.bddOne();
            break;
        case fiver::GateType::RESET:
            model[node].function = bddManager.bddZero();
            break;
        case fiver::GateType::NOT:
            model[node].function = !model[op1].function;
            break;
        case fiver::GateType::AND:
            model[node].function =  (model[op1].function & model[op2].function);
            break;
        case fiver::GateType::NAND:
            model[node].function = !(model[op1].function & model[op2].function);
            break;
        case fiver::GateType::OR:
            model[node].function =  (model[op1].function | model[op2].function);
            break;
        case fiver::GateType::NOR:
            model[node].function = !(model[op1].function | model[op2].function);
            break;
        case fiver::GateType::XOR:
            model[node].function =  (model[op1].function ^ model[op2].function);
            break;
        case fiver::GateType::XNOR:
            model[node].function = !(model[op1].function ^ model[op2].function);
            break;
        case fiver::GateType::BUF:
            model[node].function =  model[op1].function;
            break;
        case fiver::GateType::IN:
            model[node].function = bddManager.bddVar(node);
            break;
        case fiver::GateType::VDD:
            model[node].function = bddManager.bddOne();
            break;
        case fiver::GateType::GND:
            model[node].function = bddManager.bddZero();
            break;
        default:
            std::cout << "[ER-VER] Not supported gate type!" << std::endl;   
            break;
    }
}


void verify::passes::analysis::fault_node(fiver::Circuit &golden, fiver::Circuit &model, Cudd& bddManager, fiver::Gate node, Fault fault){
    // Operand gates
    fiver::Gate op1, op2;

    // Read operand function(s) from source nodes(s)
    if (unary.find(golden[node].type) != unary.end()) {
        op1 = source(*(in_edges(node, golden).first+0), golden);
    } else if (binary.find(golden[node].type) != binary.end()) {
        op1 = source(*(in_edges(node, golden).first+0), golden);
        op2 = source(*(in_edges(node, golden).first+1), golden);
    }

    // TODO: Complete list of functions in elaborate_node()...
    // Generate faulty function for gate
    switch (fault){
        case SET:
            model[node].function = bddManager.bddOne();
            model[node].type = fiver::GateType::SET;
            break;
        case RESET:
            model[node].function = bddManager.bddZero();
            model[node].type = fiver::GateType::RESET;
            break;
        case AND:
            model[node].function =  (model[op1].function & model[op2].function);
            model[node].type = fiver::GateType::AND;
            break;
        case NAND:
            model[node].function = !(model[op1].function & model[op2].function);
            model[node].type = fiver::GateType::NAND;
            break;
        case OR:
            model[node].function =  (model[op1].function | model[op2].function);
            model[node].type = fiver::GateType::OR;
            break;
        case NOR:
            model[node].function = !(model[op1].function | model[op2].function);
            model[node].type = fiver::GateType::NOR;
            break;
        case XOR:
            model[node].function =  (model[op1].function ^ model[op2].function);
            model[node].type = fiver::GateType::XOR;
            break;
        case XNOR:
            model[node].function = !(model[op1].function ^ model[op2].function);
            model[node].type = fiver::GateType::XNOR;
            break;
        case NOTA:
            model[node].function = !model[op1].function;
            model[node].type = fiver::GateType::NOT;
            break;
        case A:
            model[node].function = model[op1].function;
            model[node].type = fiver::GateType::BUF;
            break;
        default:
            std::cout << "[ERR-VER] Not suporrted fault type!" << std::endl;
            break;
    } 
}


void verify::passes::analysis::fault_injection_incremental(fiver::Circuit &golden, fiver::Circuit &model, Cudd& bddManager, std::vector<fiver::Gate> &nodes, std::vector<Fault> &faultList){
    for(int it=0; it < nodes.size(); it++){
        fault_node(golden, model, bddManager, nodes[it], faultList[it]);
        for(int pos=model[nodes[it]].propagation_path.size()-1; pos >=0; --pos){
            elaborate_node(model, bddManager, (fiver::Gate) model[nodes[it]].propagation_path[pos]);
            //if(nodes[0] == 256) std::cout << model[nodes[it]].propagation_path[pos] << std::endl;
        }

    }
}


void verify::passes::analysis::generate_fault_combinations(int num_of_faults, std::vector<std::vector<fiver::Gate>> &permutedFaultLocations, std::vector<fiver::Gate> &faultLocations){
    std::vector<bool> bitmask(faultLocations.size());
    std::fill(bitmask.begin(), bitmask.begin() + num_of_faults, true);

    do {
        std::vector<fiver::Gate> temp;
        for(int i=0; i < faultLocations.size(); ++i){
            if(bitmask[i]){
                temp.push_back(faultLocations[i]);
            }
        }
        permutedFaultLocations.push_back(temp);
    } while(std::prev_permutation(bitmask.begin(), bitmask.end()));

}


void verify::passes::analysis::generate_fault_combinations_variate(std::vector<std::vector<fiver::Gate>> &permutedFaultLocations, std::vector<std::vector<std::vector<fiver::Gate>>> &permutedFaultLocationsSeparated, int i, std::vector<fiver::Gate> new_comb){
    if (i == permutedFaultLocationsSeparated.size()){
        // new_comb can be added to permutedFaultLocations
        permutedFaultLocations.push_back(new_comb);
    } else {
         // get valid fault combinations for one logic level
        std::vector<std::vector<fiver::Gate>> col = permutedFaultLocationsSeparated[i];
        
        // loop over all valid fault combinations within one logic level
        for(int j = 0; j < col.size(); ++j){
            std::vector<fiver::Gate> tmp(new_comb);
            tmp.insert(tmp.begin(), col[j].begin(), col[j].end());
            // recursively get valid nodes from permutedFaultLocationsSeparated
            generate_fault_combinations_variate(permutedFaultLocations, permutedFaultLocationsSeparated,i+1,tmp);
        }
    }
}


std::vector<fiver::Gate> verify::passes::analysis::generate_specific_fault_combinations(std::vector<fiver::Gate> &faultLocations,  std::vector<bool> &bitmask){
    std::vector<fiver::Gate> temp;
    for(int i=0; i < faultLocations.size(); ++i){
        if(bitmask[i]){
            temp.push_back(faultLocations[i]);
        }
    }
    return temp;
}


void verify::passes::analysis::generate_fault_combinations_partly(std::vector<std::vector<fiver::Gate>> &faultLocations, std::vector<std::vector<fiver::Gate>> &permutedFaultLocations, std::vector<std::vector<bool>> &bitmasks, int num_of_faults, int variate, uint64_t start, uint64_t range){
    // Main loop to determine permutations
    for(uint64_t i=start; i < (start+range); ++i){
        // Determine subpermutations that should be used to determine the target permutation
        std::vector<fiver::Gate> temp, temp2;
        // determine valid combinations for each logic stage
        for(int v=0; v<variate; ++v){
            // get valid combination of gates
            temp = generate_specific_fault_combinations(faultLocations[v], bitmasks[v]);
            // shouldn't be empty
            if(temp.size() == 0){
                std::cerr << "[ERR-GEN-COMB] Generate fault combinations party failed!" << std::endl;
                return;
            } else {
                temp2.insert(temp2.end(), temp.begin(), temp.end());
            }
            // generate next valid permutation
            std::prev_permutation(bitmasks[v].begin(), bitmasks[v].end());
        }
        // push generated combination over all logic stages to permutedFaultLocations
        permutedFaultLocations.push_back(temp2);
    }
}

// // this function is used to evaluate dulication based countermeasures
// uint64_t verify::passes::analysis::evaluate(fiver::Circuit &golden, fiver::Circuit &faulty, int input_size, std::vector<fiver::Gate> &outputs){
//     // // Determine fault coverage for duplication countermeasure
//     BDD comp, compprime;
//     int size_original = outputs.size()/2;
//     // BDD compares original circuit with duplication
//     comp = (faulty[outputs[0]].function ^ faulty[outputs[size_original]].function);
//     // BDD compares golden circuit with faulted original and faulted duplication
//     compprime = (faulty[outputs[0]].function ^ golden[outputs[0]].function) | (faulty[outputs[size_original]].function ^ golden[outputs[0]].function);
//     // Loop over all output register
//     for (int out = 1; out < size_original; out++) {
//         comp |= (faulty[outputs[out]].function ^ faulty[outputs[out+size_original]].function);
//         compprime |= ((faulty[outputs[out]].function ^ golden[outputs[out]].function) | (faulty[outputs[out+size_original]].function ^ golden[outputs[out]].function));
//     }

//     // Add up effective fault injections
//     return (comp ^ compprime).CountMinterm(input_size);
// }

// // sbox parity
// uint64_t verify::passes::analysis::evaluate(fiver::Circuit &golden, fiver::Circuit &faulty, int input_size, std::vector<fiver::Gate> &outputs){
//     // // Determine fault coverage for duplication countermeasure
//     BDD comp, compprime;
//     int size_original = outputs.size()-1;
//     // BDD compares original circuit with duplication
//     //comp = (faulty[outputs[0]].function ^ golden[outputs[0]].function);
//     comp = faulty[outputs[0]].function;
//     // check parity bit
//     compprime = (faulty[outputs[0]].function ^ golden[outputs[0]].function);
//     // Loop over all output register
//     for (int out = 1; out < size_original; out++) {
//         // comp |= (faulty[outputs[out]].function ^ golden[outputs[out]].function);
//         comp ^= faulty[outputs[out]].function;
//         compprime |= (faulty[outputs[out]].function ^ golden[outputs[out]].function);
//     }
//     comp ^= faulty[outputs[size_original]].function;

//     // Add up effective fault injections
//     return (comp ^ compprime).CountMinterm(input_size);
// }

// CRAFT 1round 1bit
// uint64_t verify::passes::analysis::evaluate(fiver::Circuit &golden, fiver::Circuit &faulty, int input_size, std::vector<fiver::Gate> &outputs){
//     // // Determine fault coverage for duplication countermeasure
//     BDD comp, compprime;
//     int size_original = outputs.size()-1;
//     // BDD to store the error flag
//     comp = faulty[outputs[size_original]].function;
//     // compare output to golden circuit
//     compprime = (faulty[outputs[0]].function ^ golden[outputs[0]].function);
//     for (int out = 1; out < size_original; out++) {
//         compprime |= (faulty[outputs[out]].function ^ golden[outputs[out]].function);
//     }

//     // Due to the large number of input combinations, we return the logarithmic result
//     double err = std::log10((compprime & (!comp)).CountMinterm(input_size));
//     if (err <= 0){
//         return 0;   // no effective fault was detected
//     } else {
//         return (uint64_t) std::ceil(err);
//     }
// }

// Error Detection
void verify::passes::analysis::evaluate_detection(fiver::Circuit &golden, fiver::Circuit &faulty, int input_size, std::vector<fiver::Gate> &outputs, uint64_t &effective, uint64_t &ineffective, uint64_t &detected, uint64_t &scenarios){
    // // Determine fault coverage for duplication countermeasure
    BDD comp, compprime;
    int size_original = outputs.size()-1;
    // BDD to store the error flag
    comp = faulty[outputs[size_original]].function;
    // compare output to golden circuit
    compprime = (faulty[outputs[0]].function ^ golden[outputs[0]].function);
    for (int out = 1; out < size_original; out++) {
        compprime |= (faulty[outputs[out]].function ^ golden[outputs[out]].function);
    }

    // Due to the large number of input combinations, we return the logarithmic result
    double err_eff = std::log10((compprime & (!comp)).CountMinterm(input_size));
    if (err_eff <= 0){
        effective += 0;   // no effective fault was detected
    } else {
        effective += (uint64_t) std::ceil(err_eff);
    }

    double err_det = std::log10((compprime & (comp)).CountMinterm(input_size));
    if (err_det <= 0){
        detected += 0;   // no effective fault was detected
    } else {
        detected += (uint64_t) std::ceil(err_det);
    }

    scenarios += input_size;
}

// // Error Detection + TI
// void verify::passes::analysis::evaluate(fiver::Circuit &golden, fiver::Circuit &faulty, int input_size, std::vector<fiver::Gate> &outputs, uint64_t &effective, uint64_t &ineffective, uint64_t &detected, uint64_t &scenarios){
//     // // Determine fault coverage for duplication countermeasure
//     BDD comp, compprime;
//     int size_original = outputs.size()-3;
//     // BDD to store the error flag
//     comp = faulty[outputs[size_original]].function & faulty[outputs[size_original+1]].function & faulty[outputs[size_original+2]].function;
//     // compare output to golden circuit
//     compprime = (faulty[outputs[0]].function ^ golden[outputs[0]].function);
//     for (int out = 1; out < size_original; out++) {
//         compprime |= (faulty[outputs[out]].function ^ golden[outputs[out]].function);
//     }

//     // Due to the large number of input combinations, we return the logarithmic result
//     double err_eff = std::log10((compprime & (comp)).CountMinterm(input_size));
//     if (err_eff <= 0){
//         effective += 0;   // no effective fault was detected
//     } else {
//         effective += (uint64_t) std::ceil(err_eff);
//     }

//     double err_det = std::log10((compprime & (!comp)).CountMinterm(input_size));
//     if (err_det <= 0){
//         detected += 0;   // no effective fault was detected
//     } else {
//         detected += (uint64_t) std::ceil(err_det);
//     }

//     scenarios += input_size;
// }

// Error Correction
void verify::passes::analysis::evaluate_correction(fiver::Circuit &golden, fiver::Circuit &faulty, int input_size, std::vector<fiver::Gate> &outputs, uint64_t &effective, uint64_t &ineffective, uint64_t &detected, uint64_t &scenarios){
    // // Determine fault coverage for duplication countermeasure
    BDD compprime;
    compprime = (faulty[outputs[0]].function ^ golden[outputs[0]].function);
    for (int out = 1; out < outputs.size(); out++) {
        compprime |= (faulty[outputs[out]].function ^ golden[outputs[out]].function);
    }

    // Due to the large number of input combinations, we return the logarithmic result
    double err_eff = std::log10(compprime.CountMinterm(input_size));
    if (err_eff <= 0){
        effective += 0;   // no effective fault was detected
    } else {
        effective += (uint64_t) std::ceil(err_eff);
    }

    detected = 0;

    scenarios += input_size;
}


void verify::passes::analysis::fault_verification(std::vector<fiver::Circuit> &golden, std::vector<Cudd> &bddManager, int num_of_faults, int variate, std::map<fiver::GateType, std::vector<verify::passes::analysis::Fault>> &faultMap, std::vector<fiver::Gate> &validFaultLocations, std::vector<fiver::Gate> &reducedFaultLocations, int reduced_complexity, bool correction, bool interrupt){
    int num_threads = golden.size();
    
    // Find input / output nodes
    std::vector<fiver::Gate> inputs, outputs;
    for (auto gate = vertices(golden[0]).first; gate != vertices(golden[0]).second; ++gate) {
        if (golden[0][*gate].type == fiver::GateType::IN) inputs.push_back(*gate);
        if (golden[0][*gate].type == fiver::GateType::OUT) outputs.push_back(*gate);
    }

    std::cout << "Analyzing circuit with " << inputs.size() << " inputs and " << outputs.size() << " outputs." << std::endl << std::endl;

    // Variables for evaluation
    uint64_t effective = 0, ineffective = 0, detected = 0, scenarios = 0; 
    uint64_t effective_n1 = 0;
    std::vector<uint64_t> combinations_tested(num_of_faults, 0);

    // working vector for falid fault locations
    std::vector<fiver::Gate> validFaultLocationsIn;

    // open file to write results
    std::ofstream results;
    results.open("test/results.txt");
    int intermediate_results[num_threads] = {0};
    std::vector<std::string> FaultString = {"AND", "NAND", "OR", "NOR", "XOR", "XNOR", "RESET", "SET", "A", "B", "NOTA", "NOTB", "SUB", "NSUB", "CONV", "NCONV"};
    std::vector<std::string> GateString = {"IN", "GND", "VDD", "OUT", "REG", "BUF", "NOT", "AND", "NAND", "OR", "NOR", "XOR", "XNOR", "SET", "RESET"};

    // Outer loop to cover all fault injections with up to n faults
    for(int faults_per_round=1; faults_per_round <= num_of_faults; faults_per_round++){
        // Determine if complexity should be reduced
        if(faults_per_round > 1 && reduced_complexity != 0){
            if(faults_per_round == 2) effective_n1 = effective;
            if(effective_n1 == 0){
                validFaultLocationsIn = reducedFaultLocations;
            }
        }
        else {
            validFaultLocationsIn = validFaultLocations;
        }

        // Determine different stage in validFaultLocationIn
        std::vector<uint32_t> stage_numbers;
        for(auto vLoc=validFaultLocationsIn.begin(); vLoc != validFaultLocationsIn.end(); ++vLoc){
            if(find(stage_numbers.begin(), stage_numbers.end(), golden[0][*vLoc].stage_index) == stage_numbers.end()) {
                stage_numbers.push_back(golden[0][*vLoc].stage_index);
            }
        }
        std::cout << "Found " << stage_numbers.size() << " valid stages to inject faults." << std::endl << std::endl; 

        // Information
        std::cout << "Fault injections with n=" << faults_per_round << " faults per logic level and variate set to " << variate << std::endl;
        std::cout << "-------------------------------------" << std::endl;
        results << "Effective faults found for n=" << faults_per_round << std::endl;
        results << "-----------------------------------------------------------------" << std::endl;

        // create a bitmask with number of logic stages indices
        // fill with selected variate value
        std::vector<bool> bitmask(stage_numbers.size());
        std::fill(bitmask.begin(), bitmask.begin() + variate, true);

        // Handling univariate, bivariate and multivariate fault injections
        do {
            // start to fill a vector with all valid faultLocations for the current analysis
            std::vector<std::vector<fiver::Gate>> faultLocations;
            for(int i=0; i < stage_numbers.size(); ++i){
                if(bitmask[i]){
                    std::vector<fiver::Gate> temp;
                    for(auto loc=validFaultLocationsIn.begin(); loc != validFaultLocationsIn.end(); ++loc){
                        if(golden[0][*loc].stage_index == stage_numbers[i]){
                            // if logical stage i should be considered, add all corresponding gates to faultLocations
                            temp.push_back(*loc);
                        }
                    }
                    faultLocations.push_back(temp);
                }
            }

            // set number of combinations that should be computed and holded in memory
            uint64_t range = 10e6;

            // compute total number of combinations to be tested
            uint64_t num_of_combinations = 1;
            for(int v=0; v<variate; ++v){
                num_of_combinations *=  binomial_coeff((uint64_t)faultLocations[v].size(), (uint64_t)faults_per_round); 
            }

            // define bitmasks
            std::vector<std::vector<bool>> bitmasks;
            for(int var=0; var<variate; ++var){
                    std::vector<bool> bitmask(faultLocations[var].size());
                    std::fill(bitmask.begin(), bitmask.begin() + faults_per_round, true);
                    bitmasks.
                    push_back(bitmask);
            }

            for(uint64_t start=0; start<num_of_combinations; start += range){
                if(start+range > num_of_combinations){
                    range = num_of_combinations - start;
                }

                // generate list of permuted fault locations
                std::vector<std::vector<fiver::Gate>> permutedFaultLocations;
                generate_fault_combinations_partly(faultLocations, permutedFaultLocations, bitmasks, num_of_faults, variate, start, range);

                uint64_t effective_par[num_threads] = {0};
                uint64_t ineffective_par[num_threads] = {0};
                uint64_t detected_par[num_threads] = {0};
                uint64_t scenarios_par[num_threads] = {0};
                
                fiver::Circuit faulty[num_threads];
                for(int f=0; f<num_threads; ++f) faulty[f] = golden[f];

                std::cout << "Permuted fault Locations: " << permutedFaultLocations.size() << std::endl;
                combinations_tested[faults_per_round-1] += permutedFaultLocations.size();

                // Define variables to print verification progress
                unsigned int simulation_counter = 0;
                unsigned int progress_step = std::max(10, int(permutedFaultLocations.size()/100));

                bool cancel = false;
                
                // parallized loop - performs fault injection
                #pragma omp parallel for schedule(dynamic) num_threads(num_threads) shared(cancel)
                for (int location = 0; location < permutedFaultLocations.size(); location++) {  
                    #pragma omp cancellation point for                  
                    // get current thread number
                    int thread_num = omp_get_thread_num();

                    int n = faults_per_round*variate;
                    std::vector<fiver::Gate> faultNodesOld(n);
                    std::vector<Fault> faultTypesOld(n);

                    // Determine parameter to sweep over all combinations of fault types within selected gates
                    int max_comb = 1;   // max. combinations of fault types for target gates
                    int div[n];
                    int mod[n];
                    for(int th=0; th < n; th++){
                        div[th] = max_comb;
                        mod[th] = faultMap[golden[thread_num][permutedFaultLocations[location][th]].type].size();
                        max_comb *= mod[th];
                    }

                    // Sweep over all combinations
                    for(int s=0; s < max_comb; s++){
                        std::vector<fiver::Gate> faultNodes;
                        std::vector<Fault> faultTypes;

                        // Combine faulted gates with fault types
                        for(int g=0; g < n; g++){
                            int type_pos = 0;
                            if(g == 0){
                                type_pos = s % mod[0];
                            } else{
                                type_pos = (s / div[g]) % mod[g];
                            }

                            // Determine fault types and fault positions
                            bool add = true;
                            for(int search=0; search < faultNodesOld.size(); ++search){
                                if((faultNodesOld[search] == permutedFaultLocations[location][g]) && (faultTypesOld[search] == faultMap[golden[thread_num][permutedFaultLocations[location][g]].type][type_pos])){
                                    add = false;
                                }
                            }

                            if(add){
                                faultTypes.push_back(faultMap[golden[thread_num][permutedFaultLocations[location][g]].type][type_pos]);
                                faultNodes.push_back(permutedFaultLocations[location][g]);
                            
                                faultTypesOld[g] = faultMap[golden[thread_num][permutedFaultLocations[location][g]].type][type_pos];
                                faultNodesOld[g] = permutedFaultLocations[location][g];
                            }  
                            /*--------------------------------------*/               
                        }

                        fault_injection_incremental(golden[thread_num], faulty[thread_num], bddManager[thread_num], faultNodes, faultTypes);

                        // evaluation functions
                        intermediate_results[thread_num] = effective_par[thread_num];
                        if(correction){
                            evaluate_correction(golden[thread_num], faulty[thread_num], inputs.size(), outputs, effective_par[thread_num], ineffective_par[thread_num], detected_par[thread_num], scenarios_par[thread_num]);
                        } else {
                            evaluate_detection(golden[thread_num], faulty[thread_num], inputs.size(), outputs, effective_par[thread_num], ineffective_par[thread_num], detected_par[thread_num], scenarios_par[thread_num]);
                        }

                        // This code snippet can be used to print input vectors that lead to effective faults...
                        if(interrupt){
                            #pragma omp cancellation point for    
                            if(effective_par[thread_num] != 0 && cancel == false){
                                cancel = true;
                                #pragma omp critical 
                                {
                                std::vector<BDD> output_bdds;
                                // std::vector<uint64_t> mindterms_out(outputs.size());
                                float mindterms_out;
                                std::cout << "Effective faults detected: ~10^" << effective_par[thread_num] << std::endl;
                                for (int out = 0; out < outputs.size(); out++) {
                                    // mindterms_out[out] = (faulty[thread_num][outputs[out]].function ^ golden[thread_num][outputs[out]].function).CountMinterm(inputs.size());
                                    mindterms_out = std::log10((faulty[thread_num][outputs[out]].function ^ golden[thread_num][outputs[out]].function).CountMinterm(inputs.size()));
                                    if(mindterms_out != 0){
                                        std::cout << "Output bit: " << out << " in location " << location << std::endl;
                                        std::cout  << "Node(s): ";
                                        for(int r=0; r<n; ++r) std::cout << permutedFaultLocations[location][r] << " ";
                                        std::cout << std::endl;
                                        std::cout << "Fault mappings: ";
                                        for(int r=0; r<n; ++r) std::cout  << "(" << GateString[golden[thread_num][faultNodesOld[r]].type] << " -> " << FaultString[faultTypesOld[r]] << ") ";
                                        std::cout << std::endl;
                                        std::cout <<"Each line indicates the variable allocation leading to effective faults. Minus signs (-) indicate a dont_care allocation of the corresponding variable." << std::endl;
                                        // (faulty[thread_num][outputs[out]].function ^ golden[thread_num][outputs[out]].function).PrintMinterm();
                                        // (faulty[thread_num][outputs[out]].function ^ golden[thread_num][outputs[out]].function).PrintDebug(inputs.size(), 4);
                                        Cudd_PrintDebug(bddManager[thread_num].getManager(), (faulty[thread_num][outputs[out]].function ^ golden[thread_num][outputs[out]].function).getNode(), inputs.size(), 2);
                                        std::cout << std::endl;
                                        break;
                                    }
                                }
                                // cancel = true;
                                }
                                #pragma omp cancel for
                            } 
                        }

                        // write results to txt file
                        #pragma omp critical
                        {
                        if(intermediate_results[thread_num] != effective_par[thread_num]){
                            results << ">>> Effective faults in gates: ";
                            for(int r=0; r<n;++r){
                                results << permutedFaultLocations[location][r] << " ";
                            }
                            results << std::endl;
                            results << "    ";
                            for(int r=0; r<n;++r){
                                results << "(" << GateString[golden[thread_num][faultNodesOld[r]].type] << " -> " << FaultString[faultTypesOld[r]] << ") ";
                            }
                            results << std::endl;
                        }
                        }

                    }

                    // restore faulty circuit to golden model
                    // avoids copying the golden circuit for each iteration
                    for(auto r=permutedFaultLocations[location].begin(); r != permutedFaultLocations[location].end(); ++r){
                        faulty[thread_num][*r].type = golden[thread_num][*r].type;
                        elaborate_node(faulty[thread_num], bddManager[thread_num], *r);
                        for(auto n=faulty[thread_num][*r].propagation_path.end()-1; n != faulty[thread_num][*r].propagation_path.begin()-1; --n){
                            elaborate_node(faulty[thread_num], bddManager[thread_num], *n);
                        }
                    }

                    #pragma omp atomic
                    simulation_counter++;

                    // Progress
                    #pragma omp critical
                    {
                    if(((simulation_counter % progress_step) == 0) || simulation_counter == permutedFaultLocations.size()-1){
                        int barWidth = 100;
                        float progress = float(simulation_counter) / float(permutedFaultLocations.size()-1);

                        std::cout << "[";
                        int barPos = barWidth * progress;
                        for (int i = 0; i < barWidth; ++i) {
                            if (i < barPos) std::cout << "=";
                            else if (i == barPos) std::cout << ">";
                            else std::cout << " ";
                        }
                        std::cout << "] " << int(progress * 100.0) << " %\r";
                        std::cout.flush();
                    }
                    }
                }               
                
                for(int a=0; a<num_threads; ++a) effective += effective_par[a];
                for(int a=0; a<num_threads; ++a) detected += detected_par[a];
                for(int s=0; s<num_threads; ++s) scenarios += scenarios_par[s];
                //std::cout << detected << std::endl;
                std::cout << std::endl;

                if(cancel) goto exit;
            }

        } while(std::prev_permutation(bitmask.begin(), bitmask.end()));     // create all combinations with the selected variate and available logical stages

        std::cout << std::endl;
        std::cout << "Tested " << combinations_tested[faults_per_round-1] << " combinations." << std::endl << std::endl;

        ineffective = scenarios - effective - detected; 
        std::cout << ">>> found " << effective << " effective faults" << std::endl;
        std::cout << ">>> found " << ineffective << " ineffective faults" << std::endl;
        std::cout << ">>> found " << detected << " detected faults" << std::endl;
        std::cout << ">>> found " << scenarios << " fault scenarios" << std::endl << std::endl;

        results << std::endl;
    }

exit:
    uint64_t comb_total = 0;
    for(int i=0; i<num_of_faults; ++i) comb_total += combinations_tested[i];

    
    std::cout << "--------------------------------------------------------------" << std::endl;
    std::cout << "Tested " << comb_total << " combinations in total." << std::endl << std::endl;

    ineffective = scenarios - effective - detected;
    std::cout << ">>> found " << effective << " effective faults" << std::endl;
    std::cout << ">>> found " << ineffective << " ineffective faults" << std::endl;
    std::cout << ">>> found " << detected << " detected faults" << std::endl;
    std::cout << ">>> found " << scenarios << " fault scenarios" << std::endl << std::endl;

    float rate = float(effective)/float(scenarios);
    std::cout << std::setprecision(6) << ">>> fault rate: " << rate << std::endl;

    results << "-- SUMMARY -------------------------------------------------------" << std::endl;
    results << ">>> found " << effective << " effective faults" << std::endl;
    results << ">>> found " << ineffective << " ineffective faults" << std::endl;
    results << ">>> found " << detected << " detected faults" << std::endl;
    results << ">>> found " << scenarios << " fault scenarios" << std::endl;
    results << "------------------------------------------------------------------" << std::endl;

    results.close();

}