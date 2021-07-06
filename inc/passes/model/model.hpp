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

#ifndef __VERIFY_PASSES_ANALYSIS_MODEL_HPP_
#define __VERIFY_PASSES_ANALYSIS_MODEL_HPP_

#include <map>
#include <chrono>
#include <vector>

#include "../../circuit.hpp"
#include "../analysis/fault.hpp"
#include "util.hpp"

namespace verify 
{
    namespace passes
    {
        namespace model
        {

            /**
            * @brief Extracts the blacklisted entities from given file and stores them in blacklistedEntities.
            * @param file Location of the blacklisted entities.
            * @param blacklistedEntities Stores the names of the entieties that should not be faulted.
            */
            int get_blacklisted_entities(const std::string file, std::vector<std::string> &blacklistedEntities);

            /**
            * @brief Extracts the blacklisted nodes from the given model and stores them in the blacklist vector.
            * @param golden Golden circuit model.
            * @param blacklist After execution, this vector contains all nodes that where labeled as blacklisted.
            */
            int get_blacklisted_nodes(fiver::Circuit golden, std::vector<fiver::Gate> &blacklist);

            /**
            * @brief Extracts the fault mapping that should be used.
            * @param file Location of the defined fault mapping.
            * @param faultMap Stores the final fault mapping.
            */
            int get_fault_type(const std::string file, std::map<fiver::GateType, std::vector<verify::passes::analysis::Fault>> &faultMap);

            /**
            * @brief Extracts the allowed fault locations.
            * @param location Contains the location from the config file.
            * @param golden Golden circuit model.
            * @param faultLocations Contains the gates which should be faulted.
            * @param blacklist Blacklisted gates.
            */
            int get_fault_location(const std::string location, fiver::Circuit golden, std::vector<fiver::Gate> &faultLocations, std::vector<fiver::Gate> &blacklist);

            /**
            * @brief Determines a reduced set of allowed fault locations to allowe simulations for larger n.
            * @param golden Golden circuit model.
            * @param faultLocations Contains the gates which should be faulted (not reduced).
            * @param reducedFaultLocations Contains the rediced set of fault locations.
            */
            int get_reduced_fault_location_naive(fiver::Circuit &golden, std::vector<fiver::Gate> &faultLocations, std::vector<fiver::Gate> &reducedFaultLocations);

            int get_reduced_fault_location_aggrassive(fiver::Circuit &golden, std::vector<fiver::Gate> &faultLocations, std::vector<fiver::Gate> &reducedFaultLocations);

            int get_reduced_fault_location_conservative(fiver::Circuit &golden, std::vector<fiver::Gate> &faultLocations, std::vector<fiver::Gate> &reducedFaultLocations);
        }
    }
}


#endif // __VERIFY_PASSES_ANALYSIS_MODEL_HPP_
