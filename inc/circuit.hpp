/*
 * -----------------------------------------------------------------
 * COMPANY : Ruhr-Universität Bochum, Chair for Security Engineering
 * AUTHOR  : Pascal Sasdrich (pascal.sasdrich@rub.de)
 *           Jan Richter-Brockmann (jan.richter-brockmann@rub.de)
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

#ifndef __fiver_CIRCUIT_HPP_
#define __fiver_CIRCUIT_HPP_

#include "config.hpp"

#include <set>
#include <string>
#include <iostream>

#include "boost/graph/graph_traits.hpp"
#include "boost/graph/adjacency_list.hpp"

namespace fiver {

    typedef std::string GateDescription;
    typedef BDD GateFunction;
    typedef BDD GateVariables;

    typedef int ShareAnnotation;
    typedef int HierarchyAnnotation;

    enum GateType {
        IN,
        GND,
        VDD,     
        OUT,
        REG,
        BUF,
        NOT, 
        AND,
        NAND,
        OR, 
        NOR, 
        XOR, 
        XNOR,
        SET, 
        RESET
    };

    /* ----------------------------------------------------------------- */

    typedef struct {
        GateType                    type;
        GateDescription             name;
        GateFunction                function;
        GateVariables               secrets;
        GateVariables               variables;
        GateVariables               registers;
        ShareAnnotation             share_index         = -1;
        ShareAnnotation             share_domain        = -1;
        HierarchyAnnotation         depth_index         = -1;
        HierarchyAnnotation         stage_index         = -1;
        HierarchyAnnotation         partition_index     = -1;
        std::vector<std::size_t>    propagation_path;     
        bool                        blacklisted         = false;
    } GateContext;

    typedef struct {

    } WireContext;

    typedef struct {
        HierarchyAnnotation logical_depth;
        HierarchyAnnotation register_stages;
        HierarchyAnnotation stages;
    } GraphContext;

    /* ----------------------------------------------------------------- */

    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, GateContext, WireContext, GraphContext> Circuit;
    typedef boost::graph_traits<Circuit>::vertex_descriptor Gate;
    typedef boost::graph_traits<Circuit>::edge_descriptor Wire;

    /* ----------------------------------------------------------------- */

    /**
     * @brief
     */
    static std::vector<Gate> get_gates_of_types(const Circuit &model, const std::set<GateType> types)
    {
        std::vector<Gate> selection;

        for (auto gate = vertices(model).first; gate != vertices(model).second; gate++) 
            if (types.find(model[*gate].type) != types.end())
                selection.push_back(*gate);
    
        return selection;
    }

    /**
     * @brief
     */
    static void print_gate_vector(const Circuit &model, const std::vector<Gate> gates)
    {
        std::cout << "<";
        for (int n = 0; n < gates.size(); n++) {
            std::cout << model[gates[n]].type << ":" << model[gates[n]].name;
            if (n != gates.size() - 1) std::cout << ",";
        }
        std::cout << ">" << std::endl;
    }

}

#endif // __fiver_CIRCUIT_HPP_