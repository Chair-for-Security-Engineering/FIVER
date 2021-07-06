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

#ifndef __VERIFY_PASSES_ANALYSIS_FAULT_HPP_
#define __VERIFY_PASSES_ANALYSIS_FAULT_HPP_

#include <map>
#include <chrono>
#include <vector>

#include "../../circuit.hpp"

namespace verify 
{
    namespace passes
    {
        namespace analysis
        {

            /*
            * Defines faults as in the fault modeling paper.
            */
            enum Fault {
                AND,    // B0
                NAND,   // B1
                OR,     // B2
                NOR,    // B3
                XOR,    // B4
                XNOR,   // B5
                RESET,  // B6
                SET,    // B7
                A,      // B8
                B,      // B9
                NOTA,   // B10
                NOTB,   // B11
                SUB,    // B12
                NSUB,   // B13
                CONV,   // B14
                NCONV,  // B15
            };

            /**
            * @brief Identifies for each gate in model all gates that are located in the propagation path.
            * @param model Circuit model to be elaborated.
            */
            void determine_propagation_path(fiver::Circuit &model);


            /**
            * @brief Elaborates a specific node in the given circuit model.
            * @param model Circuit model to be elaborated.
            * @param node Node that should be elaborated
            */
            void elaborate_node(fiver::Circuit &model, Cudd& bddManager, fiver::Gate node);


            /**
            * @brief Faults the given node in the given circuit model. 
            * @param golden Golden circuit model.
            * @param model Circuit model to be faulted.
            * @param node Node that should be faulted.
            * @param fault Fault type that should be applied to node.
            */
            void fault_node(fiver::Circuit &golden, fiver::Circuit &model, Cudd& bddManager, fiver::Gate node, Fault fault);


            // These two functions are probably not needed anymore since fault_injection_incremental performs faster
            // fiver::Circuit fault_injection(fiver::Circuit model, std::vector<fiver::Gate> nodes, std::vector<std::string> faultList);
            // fiver::Circuit fault_injection_improved(fiver::Circuit model, std::vector<fiver::Gate> nodes, std::vector<std::string> faultList);


            /**
            * @brief Performs fault injections based on the fiven vector of nodes and faults ind faultList. 
            * @param golden Golden circuit model.
            * @param model Circuit model to be faulted.
            * @param nodes Vector of nodes that should be faulted.
            * @param faultList Vector of fault types that should be applied to the corresponding nodes given in nodes.
            */
            void fault_injection_incremental(fiver::Circuit &golden, fiver::Circuit &model, Cudd& bddManager, std::vector<fiver::Gate> &nodes, std::vector<Fault> &faultList);
            

            /**
            * @brief Generates all valid combinations of nodes that should be faulted at the same time. 
            * @param num_of_faults Total number of faults that should be injected at the same time.
            * @param permutedFaultLocations Vector with all valid combinations.
            * @param faultLocations Vector with all valid nodes that should be evaluated.
            */
            void generate_fault_combinations(int num_of_faults, std::vector<std::vector<fiver::Gate>> &permutedFaultLocations, std::vector<fiver::Gate> &faultLocations);

            /**
            * @brief Generates all valid combinations of nodes that should be faulted at the same time considering multivariate faults. 
            * @param permutedFaultLocations Final permuted fault locations.
            * @param permutedFaultLocationsSeparated Vectors with all valid combinations for each variate.
            * @param i Number of the current separated permuted fault locations.
            * @param new_comb Vector with new valid combinations which should be added to permutedFaultLocations.
            */
            void generate_fault_combinations_variate(std::vector<std::vector<fiver::Gate>> &permutedFaultLocations, std::vector<std::vector<std::vector<fiver::Gate>>> &permutedFaultLocationsSeparated, int i, std::vector<fiver::Gate> new_comb);

            /**
             * @brief Compiles the gates given by true values in bitmask.
             * @param faultLocations Contains all valid fault locations.
             * @param bitmask Boolean vector that indicates current combination that should be extracted from faultLocations.
             */
            std::vector<fiver::Gate> generate_specific_fault_combinations(std::vector<fiver::Gate> &faultLocations, std::vector<bool> &bitmask);
            
            /**
             * @brief Computes a batch of size range of valid combinations of gates that should be faulted.
             * @param faultLocations Contains all valid faultLocations separated into logic stages.
             * @param permutedFaultLocations Contains the resulting valid combinations.
             * @param bitmasks Boolean vector that stores the gates that should be selected from faultLocations
             * @param num_of_faults Number of gates that should be faulted per logic stage.
             * @param variate Univariate, bivariate, multivariate...
             * @param start Number of combintations from which the function should start to generate combinations.
             * @param range Number of combinations that should ne generated
             */
            void generate_fault_combinations_partly(std::vector<std::vector<fiver::Gate>> &faultLocations, std::vector<std::vector<fiver::Gate>> &permutedFaultLocations, std::vector<std::vector<bool>> &bitmasks, int num_of_faults, int variate, uint64_t start, uint64_t range);

            // /**
            // * @brief Evaluation function to count effective faults
            // * @param golden Golden circuit model.
            // * @param faulty Faulty circuit model.
            // * @param input_size Number of primary inputs.
            // * @param outputs Vector with all output gates that should be considered.
            // */
            // uint64_t evaluate(fiver::Circuit &golden, fiver::Circuit &faulty, int input_size, std::vector<fiver::Gate> &outputs);

            /**
            * @brief Evaluation function to count effective faults for detection-based countermeasures
            * @param golden Golden circuit model.
            * @param faulty Faulty circuit model.
            * @param input_size Number of primary inputs.
            * @param outputs Vector with all output gates that should be considered.
            */
            void evaluate_detection(fiver::Circuit &golden, fiver::Circuit &faulty, int input_size, std::vector<fiver::Gate> &outputs, uint64_t &effective, uint64_t &ineffective, uint64_t &detected, uint64_t &scenarios);

            /**
            * @brief Evaluation function to count effective faults for correction-based countermeasures
            * @param golden Golden circuit model.
            * @param faulty Faulty circuit model.
            * @param input_size Number of primary inputs.
            * @param outputs Vector with all output gates that should be considered.
            */
            void evaluate_correction(fiver::Circuit &golden, fiver::Circuit &faulty, int input_size, std::vector<fiver::Gate> &outputs, uint64_t &effective, uint64_t &ineffective, uint64_t &detected, uint64_t &scenarios);

            /**
            * @brief Perfroms fault verification for the selected parameters.
            * @param golden Golden circuit models to be elaborated.
            * @param bddManager Vector of BDD manager associated with the different models in golden.
            * @param num_of_faults Total number of faults that can be injected at the same time.
            * @param variate Univariate, bivariate, ... (1, 2, 3, ...)
            * @param faultMap Gate mapping discribing the fault model.
            * @param validFaultLocations List of gates that are defined by the fault model and that are not blacklisted.
            * @param reducedFaultLocations List of gates that need to be considered if an optimization algorithm was selected.
            * @param reduced_copmlexity Number of optimization algorithm that was selected.
            * @param correction TRUE if a correction-based countermeasure is analyzed - FALSE if a detection-based countermeasure is analyzed.
            */
            void fault_verification(std::vector<fiver::Circuit> &golden, std::vector<Cudd> &bddManager, int num_of_faults, int variate, std::map<fiver::GateType, std::vector<verify::passes::analysis::Fault>> &faultMap, std::vector<fiver::Gate> &validFaultLocations, std::vector<fiver::Gate> &reducedFaultLocations,  int reduced_complexity, bool correction, bool interrupt);


             /**
            * @brief Test fault verifaction with stored intermediate result
            *        Parts of already evaluated DAGs + BDDs are stored and will be reused
            *        For storing each DAG for n=1 this is actually slower...
            * @param golden Golden circuit model to be elaborated.
            * @param num_of_faults Total number of faults that can be injected at the same time.
            * @param faultMap Gate mapping discribing the fault model.
            * @param faultLocations Gates that are should be faulted.
            */           
            void fault_verification_inter(fiver::Circuit golden, Cudd& bddManager, int num_of_faults, std::map<std::string, std::vector<std::string>> &faultMap, std::vector<fiver::Gate> &faultLocations);


            /**
            * @brief Try to parallize the fault verification
            *        Currently this is not possible ... maybe the boost library is not thread-safe?
            * @param golden Golden circuit model to be elaborated.
            * @param num_of_faults Total number of faults that can be injected at the same time.
            * @param faultMap Gate mapping discribing the fault model.
            * @param faultLocations Gates that are should be faulted.
            */
            void fault_verification_parallel(fiver::Circuit golden, Cudd& bddManager, int num_of_faults, std::map<std::string, std::vector<std::string>> &faultMap, std::vector<fiver::Gate> &faultLocations);
        }
    }
}


#endif // __VERIFY_PASSES_ANALYSIS_FAULT_HPP_
