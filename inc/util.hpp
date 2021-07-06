/*
 * -----------------------------------------------------------------
 * COMPANY : Ruhr-Universität Bochum, Chair for Security Engineering
 * AUTHOR  : Pascal Sasdrich (pascal.sasdrich@rub.de)
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

#ifndef __UTIL_HPP_
#define __UTIL_HPP_

#include "config.hpp"

#include <set>
#include <map>
#include <cmath>
#include <chrono>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <functional>

#ifndef DOUBLE_COMPARE_THRESHOLD
#define DOUBLE_COMPARE_THRESHOLD 0.000001
#endif

#ifndef LOGN_2
#define LOGN_2 0.69314718055994530942
#endif

#define time std::chrono::high_resolution_clock::now()

/**
 * @brief Information printing.
 */
static void INFO(const std::string info, std::chrono::time_point<std::chrono::high_resolution_clock> start) {
    std::chrono::duration<double, std::ratio<1>> elapsed = time - start;
    std::cout << "[" << std::setw(10) << std::fixed << std::setprecision(3) << elapsed.count() << "] " << info;
};

/**
 * @brief Definition of supported unary and binary operations.
 */
static std::set<fiver::GateType> unary   = {fiver::GateType::OUT, fiver::GateType::REG, fiver::GateType::BUF, fiver::GateType::NOT};
static std::set<fiver::GateType> binary  = {fiver::GateType::AND, fiver::GateType::NAND, fiver::GateType::OR, fiver::GateType::NOR, fiver::GateType::XOR, fiver::GateType::XNOR};
// static std::set<std::string> unary   = {"out", "reg", "buf", "not"};
// static std::set<std::string> binary  = {"and", "nand", "or", "nor", "xor", "xnor"};
// static std::set<std::string> ternary = {"and3", "nand3", "or3", "nor3", "xor3", "xnor3"};
// static std::set<std::string> quanary = {"and4", "nand4", "or4", "nor4", "xor4", "xnor4"};
// static std::set<std::string> quinary = {"and5", "nand5", "or5", "nor5", "xor5", "xnor5"};

/**
 * @brief Definition of possible probe positions for standard and robust probing.
 */
static std::set<std::string> rprobes = {"reg", "out"};
static std::set<std::string> sprobes = {"in", "ref", "and", "nand", "or", "nor", "xor", "xnor", "and3", "nand3", "or3", "nor3", "xor3", "xnor3", "and4", "nand4", "or4", "nor4", "xor4", "xnor4", "and5", "nand5", "or5", "nor5", "xor5", "xnor5"};

/**
 * @brief Split a string into tokens based on delimiter character.
 * @param line String to be split.
 * @param delimiter Delimiter character used to split string.
 */
static std::vector<std::string> 
split(const std::string line, char delimiter) 
{
    // Variable declarations
    std::vector<std::string> tokens;
    std::string token;

    // Turn string into stream
    std::stringstream stream(line);

    // Extract tokens from line
    while(getline(stream, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

// static void
// inter_vector_combinations_and(const std::vector<std::vector<Bdd>> &intra, int offset, Bdd combination, std::vector<Bdd> &inter)
// {
//     if (offset < intra.size()) {
//         for (int idx = 0; idx < intra[offset].size(); idx++) inter_vector_combinations_and(intra, offset + 1, combination & intra[offset][idx], inter);
//     } else {
//         inter.push_back(combination);
//     }
// }

// static bool
// inter_vector_combinations_xor(const std::vector<std::vector<Bdd>> &intra, int offset, Bdd combination, int varcount)
// {
//     LACE_ME;

//     if (offset < intra.size()) {
//         bool balanced = true;
//         for (int idx = 0; idx < intra[offset].size() && balanced; idx++) 
//             balanced &= inter_vector_combinations_xor(intra, offset + 1, combination ^ intra[offset][idx], varcount);
//         if (!balanced) return false;
//     } else {
//         return (abs(bdd_satcountln(combination.GetBDD(), varcount) - varcount + 1) < DOUBLE_COMPARE_THRESHOLD);
//     }

//     return true;
// }


template <typename F>
static bool is_in(std::vector<F> vec, F e){
    for(auto elem=vec.begin(); elem!=vec.end();++elem){
        if(*elem == e){
            return true;
        }
    }
    return false;
}

template<typename T>
T binomial_coeff(T n, T k){
    T result = 1;

    if(k > n-k) k = n-k;

    for(T i=0; i < k; ++i){
        result *= (n - i);
        result /= (i + 1);
    }

    return result;
}

#endif // ___UTIL_HPP_
