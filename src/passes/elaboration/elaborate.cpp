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

#include "passes/elaboration/elaborate.hpp"

#include "boost/graph/topological_sort.hpp"

#include "util.hpp"

void fiver::passes::elaboration::elaborate(Circuit& model, Cudd& bddManager)
{

    // // Sort and store shared inputs
    // std::map<int, std::vector<Gate>> sharedInputs;
    // for (auto node = vertices(model).first; node != vertices(model).second; node++) {
    //     if (model[*node].type == IN) {
    //         sharedInputs[model[*node].share_index].push_back(*node);
    //     }
    // }

    for (auto node = vertices(model).first; node != vertices(model).second; node++) {

        //std::cout << "Node: " << *node << std::endl;

        // Read operand function(s) from source nodes(s)
        std::vector<Gate> operands = gate_read_operands(model, *node);

        // Generate result function for target node
        // std::cout << "Before gate_store_function for node " << *node << std::endl;
        gate_store_functions(model, bddManager, *node, operands);
        // std::cout << "After gate_store_function..." << std::endl;

        // Store input variable dependencies for each node
        gate_store_variables(model, bddManager, *node, operands);

        // Store register dependencies for each node
        gate_store_registers(model, bddManager, *node, operands);

    }

    /* Hierarchy details */
    // ensure that we are working on a sorted graph
    std::vector<fiver::Gate> sorted; boost::topological_sort(model, std::back_inserter(sorted));

    // First, identify the register stages - first register stage gets index 0
    for(auto node=sorted.rbegin(); node != sorted.rend(); node++){
        if(model[*node].type == REG){
            model[*node].stage_index = find_input_register_index(model, *node) + 1;
        }
        get_property(model, &GraphContext::register_stages) += (get_property(model, &GraphContext::register_stages) <= model[*node].stage_index); 
    }

    // Second, identify stage_index for all remaining gates
    for(auto node=sorted.rbegin(); node != sorted.rend(); node++){
        // find input register/primary input - considered gates belongs to the next stage
        if(model[*node].type != REG && model[*node].type != OUT) {
            model[*node].stage_index = find_output_register_index(model, *node);
            get_property(model, &GraphContext::stages) += (get_property(model, &GraphContext::stages) <= model[*node].stage_index); 
        }
    }

    // Third, label output gates
    // They get the same index as the source gate
    for(auto node=sorted.rbegin(); node != sorted.rend(); node++){
        typename boost::graph_traits<fiver::Circuit>::in_edge_iterator in_edge_start, in_edge_end;
        
        if(model[*node].type == OUT){
            boost::tie(in_edge_start, in_edge_end) = in_edges(*node, model);
            model[*node].stage_index = model[source(*in_edge_start, model)].stage_index;
        }
    }    

    //return sharedInputs;
}

int fiver::passes::elaboration::find_input_register_index(Circuit &model, Gate node){
    int res = -1; // no register detected

    typename boost::graph_traits<fiver::Circuit>::in_edge_iterator in_edge_start, in_edge_end, next;
    boost::tie(in_edge_start, in_edge_end) = in_edges(node, model);
    for(next=in_edge_start; next != in_edge_end; ++next){
        if(model[source(*next, model)].type == REG){
            return model[source(*next, model)].stage_index;   
        }

        int sub_res = find_input_register_index(model, source(*next, model));

        if(sub_res > res){
            res = sub_res;
        } 
    }

    return res;
}

int fiver::passes::elaboration::find_output_register_index(Circuit &model, Gate node){
    int stage_idx = get_property(model, &GraphContext::register_stages); // no register detected

    typename boost::graph_traits<fiver::Circuit>::out_edge_iterator out_edge_start, out_edge_end, next;
    boost::tie(out_edge_start, out_edge_end) = out_edges(node, model);
    for(next=out_edge_start; next != out_edge_end; ++next){
        if(model[target(*next, model)].type == REG){
            return model[target(*next, model)].stage_index;   
        }

        int sub_stage_idx = find_output_register_index(model, target(*next, model));

        if(sub_stage_idx < stage_idx){
            stage_idx = sub_stage_idx;
        } 
    }

    return stage_idx;
}

std::vector<fiver::Gate> 
fiver::passes::elaboration::gate_read_operands(Circuit& model, Gate node) {

    std::vector<Gate> operands;

    if (unary.find(model[node].type) != unary.end()) {
        operands.push_back(source(*(in_edges(node, model).first+0), model));
    } else if (binary.find(model[node].type) != binary.end()) {
        operands.push_back(source(*(in_edges(node, model).first+0), model));
        operands.push_back(source(*(in_edges(node, model).first+1), model));
    } else if (!(model[node].type == IN) && !(model[node].type == VDD) && !(model[node].type == GND)) {
        std::cerr << "ERR1: Unsupported node detected. (ELABORATE)" << std::endl;
    }

    return operands;
}

void
fiver::passes::elaboration::gate_store_functions(Circuit& model, Cudd& bddManager, Gate node, std::vector<Gate> operands) {
      
    if (model[node].type == IN) {
        model[node].function = bddManager.bddVar(node);
    } else if (model[node].type == GND) {
        model[node].function = bddManager.bddZero();
    } else if (model[node].type == VDD) {
        model[node].function = bddManager.bddOne();
    } else if (model[node].type == BUF) {
        model[node].function = model[operands[0]].function;
    } else if (model[node].type == REG) {
        model[node].function = model[operands[0]].function;
    } else if (model[node].type == OUT) {
        model[node].function = model[operands[0]].function;
    } else if (model[node].type == NOT) {
        model[node].function = !model[operands[0]].function;
    } else if (model[node].type == AND) {
        model[node].function = (model[operands[0]].function & model[operands[1]].function);
    } else if (model[node].type == NAND) {
        model[node].function = !(model[operands[0]].function & model[operands[1]].function);
    } else if (model[node].type == OR) {
        model[node].function = (model[operands[0]].function | model[operands[1]].function);
    } else if (model[node].type == NOR) {
        model[node].function = !(model[operands[0]].function | model[operands[1]].function);
    } else if (model[node].type == XOR) {
        model[node].function = (model[operands[0]].function ^ model[operands[1]].function);
    } else if (model[node].type == XNOR) {
        model[node].function = !(model[operands[0]].function ^ model[operands[1]].function);
    } else {
        std::cerr << "ERR2: Unsupported node detected. (ELABORATE)" << std::endl;
    }
}

void
fiver::passes::elaboration::gate_store_variables(Circuit& model, Cudd& bddManager, Gate node, std::vector<Gate> operands) {
    if (model[node].type == IN) {
        model[node].variables = bddManager.bddVar(node);
    } else if (model[node].type == GND) {
        model[node].variables = bddManager.bddZero();
    } else if (model[node].type == VDD) {
        model[node].variables = bddManager.bddOne();
    } else if (operands.size() > 0) {
        model[node].variables = bddManager.bddOne();
        for (int elem = 0; elem < operands.size(); elem++)
            model[node].variables *= model[operands[elem]].variables;
    } else {
        std::cerr << "ERR: Unsupported node detected. (ELABORATE-GATE-STORE-VARIABLES)" << std::endl;
    }
}

void
fiver::passes::elaboration::gate_store_registers(Circuit& model, Cudd& bddManager, Gate node, std::vector<Gate> operands) {       
    if (model[node].type == IN) {
        model[node].registers = bddManager.bddVar(node);
    } else if (model[node].type == GND) {
        model[node].registers = bddManager.bddZero();
    } else if (model[node].type == VDD) {
        model[node].registers = bddManager.bddOne();
    } else if (operands.size() > 0) {
        model[node].registers = bddManager.bddOne();
        for (int elem = 0; elem < operands.size(); elem++) {
            if (model[operands[elem]].type == REG)
                model[node].registers *= bddManager.bddVar(operands[elem]);
            else 
                model[node].registers *= model[operands[elem]].registers;
        }
    } else {
        std::cerr << "ERR: Unsupported node detected. (ELABORATE-GATE-STORE-REGISTERS)" << std::endl;
    }
}