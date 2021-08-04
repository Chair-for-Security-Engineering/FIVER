/*
 * -----------------------------------------------------------------
 * COMPANY : Ruhr-Universität Bochum, Chair for Security Engineering
 * AUTHOR  : Pascal Sasdrich (pascal.sasdrich@rub.de)
 * DOCUMENT: https://eprint.iacr.org/2021/936.pdf
 * -----------------------------------------------------------------
 *
 * Copyright (c) 2020, Pascal Sasdrich
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

#include "frontends/instructions/parser.hpp"

#include <iostream>

#include "util.hpp"

int
fiver::frontend::instructions::parse(const std::string filePath, Circuit& model, std::vector<std::string> blacklistedEntities)
{
    // Variable declarations
    std::vector<std::string> tokens, annotations;
    std::string line, token;

    // Open circuit description text file.
    std::ifstream description(filePath);

    // Parse description line by line
    while(std::getline(description, line))
    {
        // Split line in tokens
        tokens = split(line, ' ');

        // Add new gate to circuit graph
        Gate node = add_vertex(model);

        // Label gate with type
        if(tokens[0] == "in"){
            model[node].type = IN;
        } 
        else if(tokens[0] == "gnd"){
            model[node].type = GND;
        } 
        else if(tokens[0] == "vdd"){
            model[node].type = VDD;
        } 
        else if(tokens[0] == "out"){
            model[node].type = OUT;
        } 
        else if(tokens[0] == "reg"){
            model[node].type = REG;
        }
        else if(tokens[0] == "buf"){
            model[node].type = BUF;
        }
        else if(tokens[0] == "not"){
            model[node].type = NOT;
        }
        else if(tokens[0] == "and"){
            model[node].type = AND;
        }
        else if(tokens[0] == "nand"){
            model[node].type = NAND;
        }
        else if(tokens[0] == "or"){
            model[node].type = OR;
        }
        else if(tokens[0] == "nor"){
            model[node].type = NOR;
        }
        else if(tokens[0] == "xor"){
            model[node].type = XOR;
        }
        else if(tokens[0] == "xnor"){
            model[node].type = XNOR;
        } else {
            std::cerr << "[ERR-PARSER] Unsupported gate detected: line #" << node+1 << std::endl;
        }

        // std::cout << "[0] Node: " << node << " " << std::to_string(num_vertices(model)) << std::endl;

        // Label gate with name
        model[node].name = "line" + std::to_string(node + 1);

        // Add new signal(s) to circuit graph model
        if (unary.find(model[node].type) != unary.end()) {
            add_edge(std::stoi(tokens[1]), node, model);
        } else if (binary.find(model[node].type) != binary.end()) {
            add_edge(std::stoi(tokens[1]), node, model);
            add_edge(std::stoi(tokens[2]), node, model);
        } else if (!(tokens[0] == "in" || tokens[0] == "ref" || tokens[0] == "gnd" || tokens[0] == "vdd")) {
            std::cerr << "[ERR-PARSER] Unsupported node detected: line #" << node+1 << std::endl;
        }

        // std::cout << "[1] Node: " << node << " " << std::to_string(num_vertices(model)) << std::endl;

        // Parse annotations (for blacklist)
        if (unary.find(model[node].type) != unary.end()) {
            for(auto black=blacklistedEntities.begin(); black!=blacklistedEntities.end(); ++black){
                if(tokens[3].find(*black) != std::string::npos) model[node].blacklisted = true;
            }
        } else if (binary.find(model[node].type) != binary.end()) {
            for(auto black=blacklistedEntities.begin(); black!=blacklistedEntities.end(); ++black){
                if(tokens[4].find(*black) != std::string::npos) model[node].blacklisted = true;
            }
        }

        // std::cout << "[2] Node: " << node << " " << std::to_string(num_vertices(model)) << std::endl;
    }

    // Close circuit description text file.
    description.close();

    return 0;
}