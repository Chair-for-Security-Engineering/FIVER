/*
 * -----------------------------------------------------------------
 * COMPANY : Ruhr-Universität Bochum, Chair for Security Engineering
 * AUTHOR  : Pascal Sasdrich (pascal.sasdrich@rub.de)
 *           Jan Richter-Brockmann (jan.richter-brockmann@rub.de)
 * DOCUMENT: https://eprint.iacr.org/2020/634.pdf
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

#ifndef __fiver_PASSES_ELABORATION_ELABORATE_HPP_
#define __fiver_PASSES_ELABORATION_ELABORATE_HPP_

#include <map>
#include <vector>

#include "../../circuit.hpp"

namespace fiver 
{
    namespace passes
    {
        namespace elaboration
        {
            /**
            * @brief Elaborates a circuit model for each gate.
            * @param model Circuit model to be elaborated.
            */       
            void elaborate(Circuit& model, Cudd& bddManager);

            /**
            * @brief Provides an array of all operand gates of the current gate.
            * @param model Current circuit model.
            * @param node Current gate in circuit model.
            */       
            std::vector<Gate> gate_read_operands(Circuit& model, Gate node);

            int find_input_register_index(Circuit &model, Gate node);
            int find_output_register_index(Circuit &model, Gate node);

            void gate_store_functions(Circuit& model, Cudd& bddManager, Gate node, std::vector<Gate> operands);
            void gate_store_variables(Circuit& model, Cudd& bddManager, Gate node, std::vector<Gate> operands);
            void gate_store_registers(Circuit& model, Cudd& bddManager, Gate node, std::vector<Gate> operands);
        }
    }
}


#endif // __fiver_PASSES_ELABORATION_ELABORATE_HPP_