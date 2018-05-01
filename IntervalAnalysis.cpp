#include <iostream>
#include <cstdio>
#include <set>
#include <map>
#include <stack>
#include <string>
#include <cmath>
#include <sstream>
#include <iostream>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Constants.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;


const static int POS_LIMIT = 1000;
const static int NEG_LIMIT = -1000;

std::map<BasicBlock *, std::vector<std::map<Instruction *, std::pair<int, int>>>> context;

std::pair<int, int> init(int min, int max) {
    std::pair<int, int> var;
        if (min > max) {
            var.first = POS_LIMIT;
            var.second = NEG_LIMIT;
        }
        if (min <= NEG_LIMIT)
            var.first = NEG_LIMIT;
        else if (min >= POS_LIMIT)
            var.first = POS_LIMIT;
        else
            var.first = min;

        if (max >= POS_LIMIT)
            var.first = POS_LIMIT;
        else if (max <= NEG_LIMIT)
            var.second = NEG_LIMIT;
        else
            var.second = max;

    return var;
}

bool checkEmpty (std::pair<int, int> a) {
    if (a.first == POS_LIMIT && a.second == NEG_LIMIT) {
        return true;
    }
    return false;
}

int limitRange (int a) {
    if(a < NEG_LIMIT) {
        return NEG_LIMIT;
    }
    if(a > POS_LIMIT) {
        return POS_LIMIT;
    }
    return a;
}

std::pair<int, int> addOp(std::pair<int, int> a, std::pair<int, int> b) {
    // if (checkEmpty(a)) {
    //     return a;
    // }
    // if ( checkEmpty(b)) {
    //     return b;
    // }
    // int min = NEG_LIMIT;
    // int max = POS_LIMIT;
    // if (a.first != NEG_LIMIT && b.first != NEG_LIMIT) {
    //     min = a.first + b.first;;
    // } 
    // if (a.second != POS_LIMIT && b.second != POS_LIMIT) {
    //     max = a.second + b.second;
    // }
    // return std::make_pair(min, max);


    if (checkEmpty(a) || checkEmpty(b)) {
            return std::make_pair(POS_LIMIT, NEG_LIMIT);
        }
        int lower = a.first + b.first;
        int upper = a.second + b.second;
        if (a.first == NEG_LIMIT || b.first == NEG_LIMIT) {
            lower = NEG_LIMIT;
        }
        if (a.second == POS_LIMIT || b.second == POS_LIMIT) {
            upper = POS_LIMIT;
        }
        return std::make_pair(lower, upper);
}

std::pair<int, int> subOp(std::pair<int, int> a, std::pair<int, int> b) {
    // if (checkEmpty(a)) {
    //     return a;
    // }
    // if ( checkEmpty(b)) {
    //     return b;
    // }
    // int min = NEG_LIMIT;
    // int max = POS_LIMIT;
    // if (a.first != NEG_LIMIT && b.second != POS_LIMIT) {
    //     min = a.first - b.second;
    // }
    // if (a.second != POS_LIMIT && b.first != NEG_LIMIT) {
    //     max = a.second - b.first;
    // }
    // return std::make_pair(min, max);

    if (checkEmpty(a) || checkEmpty(b)) {
            return std::make_pair(POS_LIMIT, NEG_LIMIT);
        }
        int lower = a.first - b.second;
        int upper = a.second - b.first;
        if (a.first == NEG_LIMIT || b.second == POS_LIMIT) {
            lower = NEG_LIMIT;
        }
        if (a.second == POS_LIMIT || b.first == NEG_LIMIT) {
            upper = POS_LIMIT;
        }
        return std::make_pair(lower, upper);
}

std::pair<int, int> mulOp(std::pair<int, int> a, std::pair<int, int> b) {
    if (checkEmpty(a)) {
        return a;
    }
    if ( checkEmpty(b)) {
        return b;
    }
    std::vector<int> temp;
    temp.push_back(a.first * b.first);
    temp.push_back(a.first * b.second);
    temp.push_back(a.second * b.first);
    temp.push_back(a.second * b.second);
    return std::make_pair(*(std::min_element(temp.begin(), temp.end())), *(std::max_element(temp.begin(), temp.end())));
}

std::pair<int, int> sremOp(std::pair<int, int> a, std::pair<int, int> b) {
    if (checkEmpty(a)) {
        return a;
    }
    if ( checkEmpty(b)) {
        return b;
    }
    if (a.second == POS_LIMIT && b.second == POS_LIMIT) {
        return std::make_pair(0, POS_LIMIT);
    } else if (a.second == POS_LIMIT) {
        return std::make_pair(0, b.second - 1);
    } else if (b.second == POS_LIMIT) {
        return std::make_pair(a.second < b.first ? a.first : 0, std::max(std::abs(a.first), std::abs(a.second)));
    } else if (a.second < b.first) {
        return a;
    } else {
        return std::make_pair(0, std::min(a.second, b.second-1));
    }
}

std::pair<int, int> divOp(std::pair<int, int> a, std::pair<int, int> b){
    if(checkEmpty(a) || checkEmpty(b) || (b.first == 0 && b.second == 0)) {
        return std::make_pair(NEG_LIMIT, POS_LIMIT);
    }
    std::vector<int> temp;
    if(b.first == 0) {
        temp.push_back(a.first);
        temp.push_back(a.second);
        temp.push_back(a.first/b.second);
        temp.push_back(a.second/b.second);
    }else if(b.second == 0) {
        temp.push_back(a.first * -1);
        temp.push_back(a.second * -1);
        temp.push_back(a.first / b.first);
        temp.push_back(a.second / b.first);
    }else if(b.first < 0 && b.second > 0) {
        temp.push_back(a.first);
        temp.push_back(a.first * -1);
        temp.push_back(a.second);
        temp.push_back(a.second * -1);
        temp.push_back(a.first/b.first);
        temp.push_back(a.second/b.first);
        temp.push_back(a.first/b.second);
        temp.push_back(a.second/b.second);
    } else {
        temp.push_back(a.first/b.first);
        temp.push_back(a.second/b.first);
        temp.push_back(a.first/b.second);
        temp.push_back(a.second/b.second);
    }
    return std::make_pair(*(std::min_element(temp.begin(), temp.end())), *(std::max_element(temp.begin(), temp.end())));
}

std::string getInstructionString(Instruction &I) {
    std::string instr;
    llvm::raw_string_ostream rso(instr);
    I.print(rso);
    return instr;
}

std::pair<int, int> intervalIntersection(std::pair<int, int> interval1, std::pair<int, int> interval2) {
        if (interval1.first == POS_LIMIT || interval2.first == POS_LIMIT) {
            return std::make_pair(POS_LIMIT, NEG_LIMIT);
        } else if (interval1.first > interval2.second || interval2.first > interval1.second) {
            return std::make_pair(POS_LIMIT, NEG_LIMIT);
        } else {
            return std::make_pair(std::max(interval1.first, interval2.first), std::min(interval1.second, interval2.second));
        }
}

std::pair<int, int> intervalUnion(std::pair<int, int> v1, std::pair<int, int> v2) {
    return std::make_pair(std::min(v1.first, v2.first), std::max(v1.second, v2.second));
}

std::vector<Instruction *> intersection(std::vector<Instruction *> &v1, std::vector<Instruction *> &v2) {

    std::vector<Instruction *> v3;
    sort(v1.begin(), v1.end());
    sort(v2.begin(), v2.end());
    set_intersection(v1.begin(), v1.end(), v2.begin(), v2.end(), back_inserter(v3));
    return v3;
}

std::pair<int, int> getUnion(std::pair<int, int> v1, std::pair<int, int> v2) {
    return std::make_pair(std::min(v1.first, v2.first), std::max(v1.second, v2.second));
}

std::string getIntervalString(std::pair<int, int> var) {
    if (checkEmpty(var))
        return "EMPTY SET";
    return (var.first == NEG_LIMIT ? "NEG_LIMIT" : std::to_string(var.first)) + "-" +
            (var.second == POS_LIMIT ? "POS_LIMIT" : std::to_string(var.second));
}

/**
 * The whole concept of the program:
 * Similar to the idea of Assignment, we use an work list and iterate until the fix point.
 * For each Basic Block, the input is std::map<Instruction*, varInterval>, which means variable -> range pair.
 * For each Instruction inside the basic block, calculate the new range for the variable.
 * Till the end of the Basic Block, Which can be one of the three cases:
 * 1. Ret Statement, no successors
 * 2. Unconditional Br, one successor, no need for backward analysis.
 * 3. Conditional Br, two successors, need for backward analysis.
 * The most difficult part is the conditional Br. When conducting forward analysis till the conditional Br instruction.
 * We get the exit variable -> range pair for the block. But we need to conduct the backward analysis to get the entry set for each of the successors.
 * For unconditional Br, the exit value of the Basic Block is the entry value for the sole successor.
 * And, of course, union is still needed, to union the exit value with the existing exit value set.
 * If the union output is different from that of the last iteration, then the successors must be added into work list.
 */

/**
 * =======================================================================================================
 * -----------------------------------DECLARATION---------------------------------------------------------
 * =======================================================================================================
 */
/*
 *  for X <= Y, or X > Y, we can generalize them into:
 *  X - Y = [-INF, 0], X - Y = [1, INF].
 *
 *  ^   ^        ^     ^   ^        ^
 *  X   Y        R     X   Y        R
 */
void comp(std::pair<int, int> &intervalX, std::pair<int, int> &intervalY, std::pair<int, int> &intervalR);

void compare(Value *cmpOp1, Value *cmpOp2, std::pair<int, int> &R, std::map<Instruction *, std::pair<int, int>> &result,
             std::map<std::string, Instruction *> &instructionMap);
/**
 * Here are the analysis functions for each arithmetic operators.
 * The analysis for operators, are both forward and backward.
 * which means, we need to handle the range change for each variable due to the use of the operator / instruction, during
 * both forward analysis and backward analysis.
 */
/** analysis for alloca instruction, supports both forward and backward.*/
void analyzeAlloca(Instruction &I, std::map<Instruction *, std::pair<int, int>> &blockMap,
                   std::map<std::string, Instruction *> &instructionMap, bool backward);

/** analysis for add instruction, supports both forward and backward.*/
void analyzeAdd(Instruction &I, std::map<Instruction *, std::pair<int, int>> &blockMap,
                std::map<std::string, Instruction *> &instructionMap, bool backward);

/** analysis for sub instruction, supports both forward and backward.*/
void analyzeSub(Instruction &I, std::map<Instruction *, std::pair<int, int>> &blockMap,
                std::map<std::string, Instruction *> &instructionMap, bool backward);

/** analysis for mul instruction, supports both forward and backward.*/
void analyzeMul(Instruction &I, std::map<Instruction *, std::pair<int, int>> &blockMap,
                std::map<std::string, Instruction *> &instructionMap, bool backward);

/** analysis for srem instruction, supports both forward and backward.*/
void analyzeSrem(Instruction &I, std::map<Instruction *, std::pair<int, int>> &blockMap,
                 std::map<std::string, Instruction *> &instructionMap, bool backward);

/** analysis for store instruction, supports both forward and backward.*/
void analyzeStore(Instruction &I, std::map<Instruction *, std::pair<int, int>> &blockMap,
                  std::map<std::string, Instruction *> &instructionMap, bool backward);

/** analysis for load instruction, supports both forward and backward.*/
void analyzeLoad(Instruction &I, std::map<Instruction *, std::pair<int, int>> &blockMap,
                 std::map<std::string, Instruction *> &instructionMap, bool backward);

/** analysis for br instruction, supports both forward and backward.*/
void analyzeBr(BasicBlock *BB, Instruction &I, std::map<Instruction *, std::pair<int, int>> &blockMap,
               std::map<std::string, Instruction *> &instructionMap,
               std::map<BasicBlock *, std::map<Instruction *, std::pair<int, int>>> &result);

std::map<Instruction *, std::pair<int, int>>
joinWithContext(BasicBlock *bb, std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>> blockMap);

/**
 * The analysis entry for each basic block.
 * The input are entry variables.
 * analysisMap is the place to place the union output.
 * result is the map to place the entry value set for successors.
 */

/*
 * analyze a block
 */
bool analyzeBlock(BasicBlock *BB, std::map<Instruction *, std::pair<int, int>> &input,
                  std::map<std::string, Instruction *> &instructionMap,
                  std::map<BasicBlock *, std::map<Instruction *, std::pair<int, int>>> &analysisMap,
                  std::map<BasicBlock *, std::map<Instruction *, std::pair<int, int>>> &result);

bool analyzeBlockWithContext(BasicBlock *BB,
                             std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>> &input,
                             std::map<std::string, Instruction *> &instructionMap,
                             std::map<BasicBlock *, std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>>> &contextAnalysisMap,
                             std::map<BasicBlock *, std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>>> &result);


/**
 * =======================================================================================================
 * -----------------------------------HELPER FUNCTION-----------------------------------------------------
 * =======================================================================================================
 */
/*
 * Get the instruction name from the instruction.
 */

std::string getBasicBlockLabel(BasicBlock *BB) {
    std::string basicBlockStr;
    llvm::raw_string_ostream rso(basicBlockStr);
    rso << ">>>>Basic Block: ";
    BB->printAsOperand(rso, false);
    return basicBlockStr;
}


//helper function
std::vector<std::pair<int, int>> getOperandIntervals(Instruction &I,
                                             Value *op1,
                                             Value *op2,
                                             std::map<Instruction *, std::pair<int, int>> context,
                                             std::map<Instruction *, std::pair<int, int>> &variables,
                                             std::map<std::string, Instruction *> &instructionMap) {
    bool isInContext = context.find(&I) != context.end();
    std::pair<int, int> interval_op1;
    std::pair<int, int> interval_op2;
    if (isa<llvm::ConstantInt>(op2)) {
        auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
        auto op1_instr = instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))];
        interval_op1 = variables[op1_instr];
        interval_op2 = std::pair<int, int>(op2_val, op2_val);

    } else if (isa<llvm::ConstantInt>(op1)) {
        auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
        auto op2_instr = instructionMap[getInstructionString(*dyn_cast<Instruction>(op2))];
        interval_op1 = std::pair<int, int>(op1_val, op1_val);
        interval_op2 = variables[op2_instr];
    } else {
        auto op1_instr = instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))];
        auto op2_instr = instructionMap[getInstructionString(*dyn_cast<Instruction>(op2))];
        interval_op1 = variables[op1_instr];
        interval_op2 = variables[op2_instr];
    }
    std::vector<std::pair<int, int>> results;
    results.push_back(interval_op1);
    results.push_back(interval_op2);
    return results;
}

bool unionAndCheckChanged(std::map<Instruction *, std::pair<int, int>> &input,
                          std::map<Instruction *, std::pair<int, int>> &analysisMap) {
    // if (debug) {
    //     std::cout << ">>>>BEFORE UNION:" << std::endl;
    //     std::cout << ">>INPUT" << std::endl;
    //     for (auto &p : input) {
    //         std::cout << getInstructionString(*p.first) << " >> " << p.second.getIntervalString() << std::endl;
    //     }
    //     std::cout << ">>ANALYSIS MAP" << std::endl;
    //     for (auto &p : analysisMap) {
    //         std::cout << getInstructionString(*p.first) << " >> " << p.second.getIntervalString() << std::endl;
    //     }
    // }
    bool changed = false;
    for (auto &p : input) {
        if (analysisMap.find(p.first) == analysisMap.end()) {
            analysisMap[p.first] = p.second;
            changed = true;
        } else if (checkEmpty(analysisMap[p.first])) {
            if (!checkEmpty(p.second)) {
                analysisMap[p.first] = p.second;
                changed = true;
            }
        } else if (!(p.second <= analysisMap[p.first])) {
            analysisMap[p.first].first = limitRange(std::min(p.second.first, analysisMap[p.first].first));
            analysisMap[p.first].second = limitRange(std::max(p.second.second, analysisMap[p.first].second));
            changed = true;
        } else { ;
        }
    }
    // if (debug) {
    //     std::cout << ">>>>AFTER UNION:" << std::endl;
    //     std::cout << ">>ANALYSIS MAP" << std::endl;
    //     for (auto &p : analysisMap) {
    //         std::cout << getInstructionString(*p.first) << " >> " << p.second.getIntervalString() << std::endl;
    //     }
    // }

    return changed;
}

void cleanUpEmpty(BasicBlock *bb, std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>> &blockMap) {
    for (auto &pair : blockMap) {
        bool containsEmpty = false;

        for (auto &c : *pair.first) {
            pair.second[c.first] = intervalIntersection(c.second, pair.second[c.first]);
        }

        for(auto it = pair.second.begin(); it != pair.second.end();){
            if(it->first->getName().size() == 0 && !it->first->isUsedInBasicBlock(bb)){
                it = pair.second.erase(it);
            }else{
                ++it;
            }
        }

        for (auto &v : pair.second) {
            if (checkEmpty(v.second)) {
                containsEmpty = true;
                break;
            }
        }
        if (containsEmpty) {
            for (auto &v : pair.second) {
                v.second = std::make_pair(POS_LIMIT, NEG_LIMIT);
            }
        }
    }
}


std::map<Instruction *, std::pair<int, int>> joinWithContext(BasicBlock *bb, std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>> blockMap) {
    std::map<Instruction *, std::pair<int, int>> unionSet;
    if (blockMap.size() == 0) return unionSet;
    std::vector<Instruction *> intersetInstructionSet;
    for (auto it = blockMap.begin(); it != blockMap.end(); ++it) {
        if (it == blockMap.begin()) {
            for (auto p : it->second) intersetInstructionSet.push_back(p.first);
        } else {
            std::vector<Instruction *> tempSet;
            for (auto p : it->second) tempSet.push_back(p.first);
            intersetInstructionSet = intersection(intersetInstructionSet, tempSet);
        }
    }
    for (auto it = blockMap.begin(); it != blockMap.end(); ++it) {
        if (it == blockMap.begin()) {
            for (auto &element : intersetInstructionSet) {
                    unionSet.insert(std::make_pair(element, it->second[element]));

            }
        } else {
            for (auto &element : intersetInstructionSet) {
                    unionSet[element] = intervalUnion(unionSet[element], it->second[element]);
            }
        }
    }
    for(auto it = unionSet.begin(); it != unionSet.end();){
        if(it->first->getName().size() == 0 && !it->first->isUsedInBasicBlock(bb)){
            it = unionSet.erase(it);
        }else{
            ++it;
        }
    }
    return unionSet;
}

std::pair<int, int> getOp(std::pair<int, int> op1, std::pair<int, int> op2, std::string op) {
    if (op == "add") {
        return addOp(op1, op2);
    } else if (op == "sub") {
        return subOp(op1, op2);
    } else if (op == "mul") {
        return mulOp(op1, op2);
    } else if (op == "rem") {
        return sremOp(op1, op2);
    } else {
        return std::make_pair(POS_LIMIT, NEG_LIMIT);
    }
}


/**
 * =======================================================================================================
 * -----------------------------------PRINTING FUNCTION---------------------------------------------------
 * =======================================================================================================
 */
/*
 * print the output
 */
void printAnalysisMap(std::map<BasicBlock *, std::map<Instruction *, std::pair<int, int>>> &analysisMap,
                      std::map<std::string, Instruction *> &instructionMap);

void printBasicBlockContext(std::map<BasicBlock *, std::vector<std::map<Instruction *, std::pair<int, int>>>> context) {
    for (auto &p : context) {
        std::cout << getBasicBlockLabel(p.first) << std::endl;
        for (auto &pp : p.second) {
            std::cout << "Case:" << std::endl;
            for (auto &ppp : pp) {
                std::cout << getInstructionString(*(ppp.first)) << "  >>  " << getIntervalString(ppp.second)
                          << std::endl;
            }
        }
    }
}

void printcontextCombination(std::vector<std::map<Instruction *, std::pair<int, int>>> &contextCombination) {
    for (auto &combination : contextCombination) {
        std::cout << "================Context=============" << std::endl;
        for (auto &combination_pair : combination) {
            std::cout << getInstructionString(*(combination_pair.first)) << "  >>  "
                      << getIntervalString(combination_pair.second)
                      << std::endl;
        }
    }
}

void printAnalysisMapWithContext(
        std::map<BasicBlock *, std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>>> &contextAnalysisMap) {
    for (auto &pair : contextAnalysisMap) {
        std::cout << getBasicBlockLabel(pair.first) << "===============================" << std::endl;
        // if(debug){
        //     for (auto &context_variables : pair.second) {
        //         bool isNotEmpty = true;
        //         for (auto &context_pair : *context_variables.first) {
        //             if (context_pair.second.isEmpty()) {
        //                 isNotEmpty = false;
        //                 break;
        //             }
        //         }
        //         for (auto &variable_pair : context_variables.second) {
        //             if (variable_pair.second.isEmpty()) {
        //                 isNotEmpty = false;
        //                 break;
        //             }
        //         }
        //         if (!isNotEmpty) {
        //             continue;
        //         }
        //         std::cout << "Context:  ------------------------" << std::endl;
        //         for (auto &context_pair : *context_variables.first) {
        //             std::cout << getInstructionString(*context_pair.first) << ">>"
        //                       << context_pair.second.getIntervalString() << std::endl;
        //         }
        //         std::cout << "Variables: ------------------------" << std::endl;
        //         for (auto &variable_pair : context_variables.second) {
        //             std::cout << getInstructionString(*variable_pair.first) << ">>"
        //                       << variable_pair.second.getIntervalString() << std::endl;
        //         }
        //     }
        // }


        std::map<Instruction *, std::pair<int, int>> final_result;
        final_result = joinWithContext(pair.first, pair.second);
        // if(debug)
        //     std::cout << "Final Values ------------------------" << std::endl;
        bool isNotEmpty = true;
        for (auto &pair : final_result) {
            if (checkEmpty(pair.second)) {
                isNotEmpty = false;
                break;
            }
        }
        if (!isNotEmpty) {
            continue;
        }
        for (auto &pair : final_result) {
            if(pair.first->getName().size()!=0)
                std::cout << getInstructionString(*pair.first) << "  -> " << getIntervalString(pair.second)
                      << std::endl;
        }
    }
}

// void printAnalysisMap(std::map<BasicBlock *, std::map<Instruction *, std::pair<int, int>>> &analysisMap,
//                       std::map<std::string, Instruction *> &instructionMap) {
//     std::cout << "==============Analysis Report==============" << std::endl;
//     for (auto &m : analysisMap) {
//         std::cout << getBasicBlockLabel(m.first) << std::endl;
//         for (auto &_m : m.second) {
//             std::cout << getInstructionString(*(_m.first)) << "  >>  " << getIntervalString(_m.second) << std::endl;
//         }
//         for (auto mm = m.second.begin(); mm != m.second.end(); mm++) {
//             std::string temp_mm = (*mm).first->getName().str().c_str();
//             if (temp_mm.size() == 0) {
//                 continue;
//             }
//             for (auto mm1 = mm; mm1 != m.second.end(); mm1++) {
//                 std::string temp_mm1 = (*mm1).first->getName().str().c_str();
//                 if (temp_mm1.size() == 0 || temp_mm == temp_mm1) {
//                     continue;
//                 }
//                 int v1_range[2] = {(*mm).second.first, (*mm).second.second};
//                 int v2_range[2] = {(*mm1).second.first, (*mm1).second.second};
//                 if (v1_range[0] == NEG_LIMIT || v1_range[1] == POS_LIMIT) {
//                     std::cout << temp_mm << " <--> " << temp_mm1 << " : "
//                               << "INF" << std::endl;
//                 } else if (v2_range[0] == NEG_LIMIT || v2_range[1] == POS_LIMIT) {
//                     std::cout << temp_mm << " <--> " << temp_mm1 << " : "
//                               << "INF" << std::endl;
//                 } else {
//                     int a = std::abs(v1_range[0] - v2_range[1]);
//                     int b = std::abs(v1_range[1] - v2_range[0]);
//                     std::cout << temp_mm << " <--> " << temp_mm1 << " : "
//                               << (a > b ? (a >= POS_LIMIT ? "INF" : std::to_string(a)) : (b >=
//                                                                                                      POS_LIMIT
//                                                                                                      ? "INF"
//                                                                                                      : std::to_string(
//                                               b)))
//                               << std::endl;
//                 }

//             }
//         }
//         std::cout << "===========================================" << std::endl;
//     }
// }


void generateAnalysisReport(std::stack<std::pair<BasicBlock *, std::map<Instruction *, std::pair<int, int>>>> &traversalStack,
                            std::map<BasicBlock *, std::map<Instruction *, std::pair<int, int>>> &analysisMap,
                            std::map<std::string, Instruction *> &instructionMap) {
    int count = 0;
    while (!traversalStack.empty() && count < 10000) {
        std::map<BasicBlock *, std::map<Instruction *, std::pair<int, int>>> result;
        auto pair = traversalStack.top();
        traversalStack.pop();
        // if (debug) {
        //     std::cout << "==============BLOCK ANALYSIS " << getBasicBlockLabel(pair.first) << "============="
        //               << std::endl;
        //     std::cout << ">>>>INPUT:" << std::endl;
        //     for (auto &p : pair.second) {
        //         std::cout << getInstructionString(*(p.first)) << "    " << std::endl;
        //         p.second.printIntervals();
        //     }
        //     std::cout << ">>>>PROCESSING:" << std::endl;
        // }

        auto changed = analyzeBlock(pair.first, pair.second, instructionMap, analysisMap, result);


        if (changed) {
            for (auto &p : result) {
                traversalStack.push(p);
            }
        }

        count++;

        // if (debug) {
        //     std::cout << ">>>>OUTPUT:" << std::endl;
        //     for (auto &p : analysisMap[pair.first]) {
        //         std::cout << getInstructionString(*(p.first)) << "    " << std::endl;
        //         p.second.printIntervals();
        //     }
        //     std::cout << ">>>>SUCCESSORS:" << std::endl;
        //     for (auto &p : result) {
        //         std::cout << getBasicBlockLabel(p.first) << std::endl;
        //         for (auto &pp : p.second) {
        //             std::cout << getInstructionString(*(pp.first)) << "    " << std::endl;
        //             pp.second.printIntervals();
        //         }
        //     }
        //     std::cout << "================================================================" << std::endl;
        // }


        //to pause for each iteration during debug.
        // if (pause) {
        //     std::cin.get();
        // }
    }
}


void generateContext(std::map<BasicBlock *, std::vector<std::map<Instruction *, std::pair<int, int>>>> &context,
                     std::map<std::string, Instruction *> &instructionMap,
                     std::map<BasicBlock *, std::map<Instruction *, std::pair<int, int>>> &analysisMap) {
    for (auto &p : analysisMap) {
        /**
         * TEST PART, MAY NOT WORK
         */
        for (auto &pair : p.second) {
            pair.second = std::make_pair(NEG_LIMIT, POS_LIMIT);
        }
        /**
         * TEST PART END HERE
         */

        BasicBlock *BB = p.first;
        std::map<Instruction *, std::pair<int, int>> bbOutput = p.second;
        std::map<BasicBlock *, std::map<Instruction *, std::pair<int, int>>> result;
        /**
         * for each basic block, find the br instruction
         */
        for (auto &I: *BB) {
            if (I.getOpcode() == Instruction::Br) {
                /**
                 * analyze the block backward
                 */
                analyzeBr(BB, I, bbOutput, instructionMap, result);
                /**
                 * if ret, or unconditional br, ignore
                 */
                if (result.size() <= 1)
                    break;
                /**
                 * for conditional br, record the result
                 */
                for (auto &pp : result) {
                    context[BB].push_back(pp.second);
                }
                /**
                 * for two conditions, remove the variables, that have idential varInterval in two conditions
                 */
                for (auto it_0 = context[BB][0].begin(); it_0 != context[BB][0].end();) {
                    bool broke = false;
                    for (auto it_1 = context[BB][1].begin(); it_1 != context[BB][1].end();) {
                        if (getIntervalString(it_0->second) == getIntervalString(it_1->second)) {
                            it_0 = context[BB][0].erase(it_0);
                            it_1 = context[BB][1].erase(it_1);
                            broke = true;
                            break;
                        }
                        ++it_1;
                    }
                    if (!broke)
                        ++it_0;
                }
                for (auto it_0 = context[BB][0].begin(); it_0 != context[BB][0].end();) {
                    if (it_0->first->getName().size() == 0) {
                        it_0 = context[BB][0].erase(it_0);
                    } else {
                        ++it_0;
                    }
                }
                for (auto it_1 = context[BB][1].begin(); it_1 != context[BB][1].end();) {
                    if (it_1->first->getName().size() == 0) {
                        it_1 = context[BB][1].erase(it_1);
                    } else {
                        ++it_1;
                    }
                }
            }
        }
    }
}

void generateContextCombination(std::vector<std::map<Instruction *, std::pair<int, int>>> &contextCombination,
                                std::map<BasicBlock *, std::vector<std::map<Instruction *, std::pair<int, int>>>> &context) {
    /**
     * context: pair(BasicBlock*, std::vector<std::map<Instruction*, varInterval>>)
     * >>>>Basic Block: %2
     * Case:
     *  %N = alloca i32, align 4  >>  1-INF_POS
     *  %5 = load i32* %N, align 4  >>  1-INF_POS
     *Case:
     *  %N = alloca i32, align 4  >>  INF_NEG-0
     *  %5 = load i32* %N, align 4  >>  INF_NEG-0
     *>>>>Basic Block: %7
     *Case:
     *  %a = alloca i32, align 4  >>  1-6
     *  %8 = load i32* %a, align 4  >>  1-6
     *Case:
     *  %a = alloca i32, align 4  >>  -5-0
     *  %8 = load i32* %a, align 4  >>  -5-0
     */
    //for each context
    for (auto context_pair : context) {
        /**
         * context_pair: pair(BasicBlock*, std::vector<std::map<Instruction*, varInterval>>)
         * key:
         * Basic Block: %2
         * Values:
         *  %N = alloca i32, align 4  >>  1-INF_POS
         *  %5 = load i32* %N, align 4  >>  1-INF_POS
         *  ----
         *  %N = alloca i32, align 4  >>  INF_NEG-0
         *  %5 = load i32* %N, align 4  >>  INF_NEG-0
         */
        if (context_pair.second.size() == 0 || context_pair.second[0].size() == 0)
            continue;

        if (contextCombination.size() == 0) {
            contextCombination = context_pair.second;
            continue;
        }
        auto contextCombinationBackUp = contextCombination;
        contextCombination.clear();
        //for each case
        for (auto context_pair_map : context_pair.second) {
            /**
            * context_pair_map: std::map<Instruction*, varInterval>)
            *  %N = alloca i32, align 4  >>  INF_NEG-0
            *  %5 = load i32* %N, align 4  >>  INF_NEG-0
            */
            for (auto temp_map : contextCombinationBackUp) {
                /**
                 * temp_pair: std::map<Instruction *, varInterval>
                 */
                for (auto it = context_pair_map.begin(); it != context_pair_map.end(); ++it) {
                    if (temp_map.find(it->first) != temp_map.end()) {
                        temp_map.insert(std::make_pair(it->first,
                                                       intervalIntersection(temp_map.find(it->first)->second,
                                                                                    it->second)));
                    } else {
                        temp_map.insert(*it);
                    }
                }
                contextCombination.push_back(temp_map);
            }
        }
    }
}

void runBlockIntervalAnalysis(BasicBlock *entryBB,
                              std::vector<std::map<Instruction *, std::pair<int, int>>> &contextCombination,
                              std::map<std::string, Instruction *> &instructionMap,
                              std::map<BasicBlock *, std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>>> &contextAnalysisMap,
                              std::stack<std::pair<BasicBlock *, std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>>>> &contextTraversalStack) {
    std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>> initialSet;
    for (auto &combination : contextCombination) {
        std::map<Instruction *, std::pair<int, int>> emptySet;
        initialSet.insert(std::make_pair(&combination, combination));
    }
    contextTraversalStack.push(std::make_pair(entryBB, initialSet));

    int count = 0;
    while (!contextTraversalStack.empty() && count < 10000) {
        std::map<BasicBlock *, std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>>> result;
        auto pair = contextTraversalStack.top();
        contextTraversalStack.pop();

        auto changed = analyzeBlockWithContext(pair.first, pair.second, instructionMap, contextAnalysisMap, result);

        if (changed) {
            for (auto &p : result) {
                contextTraversalStack.push(p);
            }
        }
        count++;
    }
}


/**
 * =======================================================================================================
 * -----------------------------------MAIN----------------------------------------------------------------
 * =======================================================================================================
 */

//main function
int main(int argc, char **argv) {
    //Preparation
    LLVMContext &Context = getGlobalContext();
    SMDiagnostic Err;
    Module *M = ParseIRFile(argv[1], Err, Context);
    if (M == nullptr) {
        fprintf(stderr, "error: failed to load LLVM IR file \"%s\"", argv[1]);
        return EXIT_FAILURE;
    }
    Function *F = M->getFunction("main");
    BasicBlock *entryBB = &F->getEntryBlock();

    //CONSTRUCT All Data Structures
    std::map<BasicBlock *, std::map<Instruction *, std::pair<int, int>>> analysisMap;
    std::map<std::string, Instruction *> instructionMap;
    std::stack<std::pair<BasicBlock *, std::map<Instruction *, std::pair<int, int>>>> traversalStack;
    std::map<Instruction *, std::pair<int, int>> emptySet;
    traversalStack.push(std::make_pair(entryBB, emptySet));
    //Generate Analysis Map
    generateAnalysisReport(traversalStack, analysisMap, instructionMap);
    //printAnalysisMap(analysisMap, instructionMap);

    /**
     * for each basic block, generate the context
     */
    generateContext(context, instructionMap, analysisMap);
    /**
     * finish generating the context for each block
     */
//    printBasicBlockContext(context);

    /**
     * generates different context combinations
     */
    std::vector<std::map<Instruction *, std::pair<int, int>>> contextCombination;
    generateContextCombination(contextCombination, context);
//    printcontextCombination(contextCombination);

    //CONSTRUCT All Data Structures
    std::map<BasicBlock *, std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>>> contextAnalysisMap;
    std::stack<std::pair<BasicBlock *, std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>>>> contextTraversalStack;

    runBlockIntervalAnalysis(entryBB, contextCombination, instructionMap, contextAnalysisMap, contextTraversalStack);
    printAnalysisMapWithContext(contextAnalysisMap);
}


/**
 * =======================================================================================================
 * -----------------------------------CONSTRAINT-BASED INTERVAL ANALYSIS----------------------------------
 * =======================================================================================================
 */
void operatorWithContext(BasicBlock *bb, Instruction &I,
                         std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>> &blockMap,
                         std::map<std::string, Instruction *> &instructionMap, std::string op) {
    // for(auto &pair : blockMap) {
    //     std::cout << "=======================bloack start======================" <<std::endl;
    //     for(auto &i : pair.second) {
    //         std::cout << getInstructionString(*(i.first)) << std::endl;
    //         std::cout << i.second.getIntervalString() << std::endl;
    //     }
    //     std::cout << "========================block end========================" <<std::endl;
    // }

    if (blockMap.begin()->first->find(&I) != blockMap.begin()->first->end()) {
        auto join = joinWithContext(bb, blockMap);
        for (auto &pair :blockMap) {
            auto context1 = *pair.first;
            auto result = getOperandIntervals(I, I.getOperand(0), I.getOperand(1), *pair.first, join, instructionMap);
            std::pair<int, int> temp = getOp(result[0], result[1], op);
            if (checkEmpty(intervalIntersection(temp, context1[&I])))
                for (auto &var : pair.second)
                    var.second = std::make_pair(POS_LIMIT, NEG_LIMIT);
            else {
                pair.second = join;
                pair.second[&I] = intervalIntersection(temp, context1[&I]);
            }
        }
    } else {
        for (auto &pair : blockMap) {
            auto context1 = *pair.first;
            auto result = getOperandIntervals(I, I.getOperand(0), I.getOperand(1), *pair.first, pair.second,
                                              instructionMap);
            std::pair<int, int> temp = getOp(result[0], result[1], op);
            pair.second[&I] = temp;
        }
    }
    cleanUpEmpty(bb, blockMap);
}

//operators with context
void analyzeAddWithContext(BasicBlock *bb, Instruction &I,
                           std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>> &blockMap,
                           std::map<std::string, Instruction *> &instructionMap) {
    operatorWithContext(bb, I, blockMap, instructionMap, "add");
}

void analyzeSubWithContext(BasicBlock *bb, Instruction &I,
                           std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>> &blockMap,
                           std::map<std::string, Instruction *> &instructionMap) {
    operatorWithContext(bb, I, blockMap, instructionMap, "sub");
}

void analyzeMulWithContext(BasicBlock *bb, Instruction &I,
                           std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>> &blockMap,
                           std::map<std::string, Instruction *> &instructionMap) {
    operatorWithContext(bb, I, blockMap, instructionMap, "mul");
}

void analyzeSremWithContext(BasicBlock *bb, Instruction &I,
                            std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>> &blockMap,
                            std::map<std::string, Instruction *> &instructionMap) {
    operatorWithContext(bb, I, blockMap, instructionMap, "rem");
}


void analyzeStoreWithContext(BasicBlock *bb, Instruction &I,
                             std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>> &blockMap,
                             std::map<std::string, Instruction *> &instructionMap) {
    Value *op1 = I.getOperand(0);
    auto op2_instr = instructionMap[getInstructionString(*dyn_cast<Instruction>(I.getOperand(1)))];
    std::pair<int, int> temp;
    if (blockMap.begin()->first->find(op2_instr) != blockMap.begin()->first->end()) {
        auto join = joinWithContext(bb, blockMap);
        if (isa<llvm::ConstantInt>(op1)) {
            auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
            temp = std::make_pair(op1_val, op1_val);
        } else {
            auto op1_instr = instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))];
            temp = join[op1_instr];
        }
        for (auto &pair :blockMap) {
            auto context = *pair.first;
            if (checkEmpty(intervalIntersection(temp, context[op2_instr]))) {
                for (auto &var : pair.second)
                    var.second = std::make_pair(POS_LIMIT, NEG_LIMIT);
            } else {
                pair.second = join;
                pair.second[op2_instr] = intervalIntersection(temp, context[op2_instr]);
            }
        }

    } else {
        for (auto &pair : blockMap) {
            auto context = *pair.first;
            if (isa<llvm::ConstantInt>(op1)) {
                auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
                temp = std::make_pair(op1_val, op1_val);
            } else {
                auto op1_instr = instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))];
                temp = pair.second[op1_instr];
            }
            pair.second[op2_instr] = temp;
        }
    }
    cleanUpEmpty(bb, blockMap);
}

void analyzeLoadWithContext(BasicBlock *bb, Instruction &I,
                            std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>> &blockMap,
                            std::map<std::string, Instruction *> &instructionMap) {
    auto *op_instruction = dyn_cast<Instruction>(I.getOperand(0));
    if (blockMap.begin()->first->find(&I) != blockMap.begin()->first->end()) {
        auto join = joinWithContext(bb, blockMap);
        for (auto &pair :blockMap) {
            auto context = *pair.first;
            std::pair<int, int> temp = pair.second[instructionMap[getInstructionString(*op_instruction)]];
            if (checkEmpty(intervalIntersection(temp, context[&I]))) {
                for (auto &var : pair.second)
                    var.second = std::make_pair(POS_LIMIT, NEG_LIMIT);
            } else {
                pair.second = join;
                pair.second[&I] = intervalIntersection(temp, context[&I]);
            }
        }
    } else {
        for (auto &pair : blockMap) {
            pair.second[&I] = pair.second[instructionMap[getInstructionString(*op_instruction)]];
        }
    }
    cleanUpEmpty(bb, blockMap);
}

void analyzeAllocaWithContext(BasicBlock *bb, Instruction &I,
                              std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>> &blockMap,
                              std::map<std::string, Instruction *> &instructionMap) {
    if (blockMap.begin()->first->find(&I) != blockMap.begin()->first->end()) {
        for (auto &pair :blockMap)
            pair.second = joinWithContext(bb, blockMap);
    } else {
        for (auto &pair : blockMap)
            pair.second[&I] = std::make_pair(NEG_LIMIT, POS_LIMIT);
    }
    cleanUpEmpty(bb, blockMap);

}

void analyzeBrWithContext(BasicBlock *BB, Instruction &I,
                          std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>> &blockMap,
                          std::map<std::string, Instruction *> &instructionMap,
                          std::map<BasicBlock *, std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>>> &result) {
    auto *branchInst = dyn_cast<BranchInst>(&I);
    auto *bb_op1 = branchInst->getSuccessor(0);
    if (!(branchInst->isConditional())) {
        result[bb_op1] = blockMap;
        return;
    }
    auto *bb_op2 = dyn_cast<BasicBlock>(branchInst->getSuccessor(1));

    std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>> total_branch1 = blockMap;
    std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>> total_branch2 = blockMap;


    auto branch_1_context = context[BB][0];
    auto branch_2_context = context[BB][1];


    for (auto &pair : total_branch1) {
        bool emptyBranch1 = false;
        for (auto &context_pair : *pair.first) {
            if (branch_1_context.find(context_pair.first) != branch_1_context.end() &&
                checkEmpty(intervalIntersection(branch_1_context[context_pair.first], context_pair.second))) {
                emptyBranch1 = true;
                break;
            }
        }
        if (emptyBranch1) {
            for (auto &variable_pair : pair.second) {
                variable_pair.second = std::make_pair(POS_LIMIT, NEG_LIMIT);
            }
        }
    }

    for (auto &pair : total_branch2) {
        bool emptyBranch2 = false;
        for (auto &context_pair : *pair.first) {
            if (branch_2_context.find(context_pair.first) != branch_2_context.end() &&
                checkEmpty(intervalIntersection(branch_2_context[context_pair.first], context_pair.second))) {
                emptyBranch2 = true;
                break;
            }
        }
        if (emptyBranch2) {
            for (auto &variable_pair : pair.second) {
                variable_pair.second = std::make_pair(POS_LIMIT, NEG_LIMIT);
            }
        }
    }

    result[bb_op1] = total_branch1;
    result[bb_op2] = total_branch2;

}


bool unionAndCheckChangedWithContext(
        std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>> &input,
        std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>> &analysisMap) {
    bool changed = false;
    for (auto &context : input) {
        /**
         * Each context
         */
        auto input_result = context.second;
        auto analysisMap_result = analysisMap[context.first];
        for (auto &p : input_result) {
            if (analysisMap_result.find(p.first) == analysisMap_result.end()) {
                analysisMap[context.first][p.first] = p.second;
                changed = true;
            } else if (checkEmpty(analysisMap_result[p.first])) {
                if (!checkEmpty(p.second)) {
                    analysisMap[context.first][p.first] = p.second;
                    changed = true;
                }
            } else if (!(p.second <= analysisMap_result[p.first])) {
                analysisMap[context.first][p.first].first = limitRange(
                        std::min(p.second.first, analysisMap_result[p.first].first));
                analysisMap[context.first][p.first].second = limitRange(
                        std::max(p.second.second, analysisMap_result[p.first].second));
                changed = true;
            } else { ;
            }
        }
    }
    return changed;
}

bool analyzeBlockWithContext(BasicBlock *BB,
                             std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>> &input,
                             std::map<std::string, Instruction *> &instructionMap,
                             std::map<BasicBlock *, std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>>> &contextAnalysisMap,
                             std::map<BasicBlock *, std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>>> &result) {
    for (auto &I: *BB) {
        std::cout << getInstructionString(I) << std::endl;
        switch (I.getOpcode()) {
            case Instruction::Add: {
                analyzeAddWithContext(BB, I, input, instructionMap);
                break;
            }
            case Instruction::Sub: {
                analyzeSubWithContext(BB, I, input, instructionMap);
                break;
            }
            case Instruction::Mul: {
                analyzeMulWithContext(BB, I, input, instructionMap);
                break;
            }
            case Instruction::SRem: {
                analyzeSremWithContext(BB, I, input, instructionMap);
                break;
            }
            case Instruction::Alloca: {
                analyzeAllocaWithContext(BB, I, input, instructionMap);
                break;
            }
            case Instruction::Store: {
                analyzeStoreWithContext(BB, I, input, instructionMap);
                break;
            }
            case Instruction::Load: {
                analyzeLoadWithContext(BB, I, input, instructionMap);
                break;
            }
            case Instruction::ICmp: {
                break;
            }
            case Instruction::Br: {
                analyzeBrWithContext(BB, I, input, instructionMap, result);
                bool changed = unionAndCheckChangedWithContext(input, contextAnalysisMap[BB]);
                return changed;
            }
            case Instruction::Ret: {
                bool changed = unionAndCheckChangedWithContext(input, contextAnalysisMap[BB]);
                return changed;
            }
            default: {
                std::cout << "Unknown: " << I.getOpcodeName() << std::endl;
                break;
            }
        }
    }
    return false;


}


/**
 * =======================================================================================================
 * -----------------------------------ORIGINAL DIFFERENCE ANALYSIS----------------------------------------
 *  =======================================================================================================
 */
bool analyzeBlock(BasicBlock *BB, std::map<Instruction *, std::pair<int, int>> &input,
                  std::map<std::string, Instruction *> &instructionMap,
                  std::map<BasicBlock *, std::map<Instruction *, std::pair<int, int>>> &analysisMap,
                  std::map<BasicBlock *, std::map<Instruction *, std::pair<int, int>>> &result) {
    for (auto &I: *BB) {
        std::string instructionStr = getInstructionString(I);
        instructionMap[instructionStr] = &I;
        switch (I.getOpcode()) {
            case Instruction::Add: {
                analyzeAdd(I, input, instructionMap, false);
                break;
            }
            case Instruction::Sub: {
                analyzeSub(I, input, instructionMap, false);
                break;
            }
            case Instruction::Mul: {
                analyzeMul(I, input, instructionMap, false);
                break;
            }
            case Instruction::SRem: {
                analyzeSrem(I, input, instructionMap, false);
                break;
            }
            case Instruction::Alloca: {
                analyzeAlloca(I, input, instructionMap, false);
                break;
            }
            case Instruction::Store: {
                analyzeStore(I, input, instructionMap, false);
                break;
            }
            case Instruction::Load: {
                analyzeLoad(I, input, instructionMap, false);
                break;
            }
            case Instruction::ICmp: {
                break;
            }
            case Instruction::Br: {
                analyzeBr(BB, I, input, instructionMap, result);
                return unionAndCheckChanged(input, analysisMap[BB]);
            }
            case Instruction::Ret: {
                return unionAndCheckChanged(input, analysisMap[BB]);
            }
            default: {
                std::cout << "Unknown: " << I.getOpcodeName() << std::endl;
                break;
            }
        }
    }
    return false;
}

//------OPERATOR FUNCTION IMPLEMENTATION
void analyzeAdd(Instruction &I, std::map<Instruction *, std::pair<int, int>> &blockMap,
                std::map<std::string, Instruction *> &instructionMap, bool backward) {
    std::string instructionName = getInstructionString(I);
    Value *op1 = I.getOperand(0);
    Value *op2 = I.getOperand(1);

    if (!backward) {
        if (isa<llvm::ConstantInt>(op2)) {
            auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
            std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
            std::pair<int, int> interval_op1 = blockMap[instructionMap[op1_str]];
            blockMap[instructionMap[instructionName]] = addOp(interval_op1, std::make_pair(op2_val, op2_val));
        } else if (isa<llvm::ConstantInt>(op1)) {
            auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
            std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
            std::pair<int, int> interval_op2 = blockMap[instructionMap[op2_str]];
            blockMap[instructionMap[instructionName]] = addOp(interval_op2, std::make_pair(op1_val, op1_val));
        } else {
            std::pair<int, int> interval_op1 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))]];
            std::pair<int, int> interval_op2 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op2))]];
            blockMap[instructionMap[instructionName]] = addOp(interval_op1, interval_op2);
        }
        // if (debug) {
        //     std::cout << instructionName << " ===>" << blockMap[instructionMap[instructionName]].getIntervalString()
        //               << std::endl;
        // }
    } else {
        auto instructionInterval = blockMap[instructionMap[instructionName]];
        if (isa<llvm::ConstantInt>(op2)) {
            auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
            std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
            blockMap[instructionMap[op1_str]] = subOp(instructionInterval, std::make_pair(op2_val, op2_val));
        } else if (isa<llvm::ConstantInt>(op1)) {
            auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
            std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
            blockMap[instructionMap[op2_str]] = subOp(instructionInterval, std::make_pair(op1_val, op1_val));
        } else {
            int lowerInstruction = instructionInterval.first;// - op1_val;
            int upperInstruction = instructionInterval.second;// - op1_val;
            std::string op1String = getInstructionString(*dyn_cast<Instruction>(op1));
            auto op1Interval = blockMap[instructionMap[op1String]];
            std::string op2String = getInstructionString(*dyn_cast<Instruction>(op2));
            auto op2Interval = blockMap[instructionMap[op2String]];
            int lowerOp1 = op1Interval.first;
            int upperOp1 = op1Interval.second;
            int lowerOp2 = op2Interval.first;
            int upperOp2 = op2Interval.second;
            int newLowerOp1 = std::max(lowerOp1, lowerInstruction - upperOp2);
            int newUpperOp1 = std::min(upperOp1, upperInstruction - lowerOp2);
            int newLowerOp2 = std::max(lowerOp2, lowerInstruction - upperOp1);
            int newUpperOp2 = std::min(upperOp2, upperInstruction - lowerOp1);
            blockMap[instructionMap[op1String]] = std::make_pair(newLowerOp1, newUpperOp1);
            blockMap[instructionMap[op2String]] = std::make_pair(newLowerOp2, newUpperOp2);
        }
    }
}

void analyzeSub(Instruction &I, std::map<Instruction *, std::pair<int, int>> &blockMap,
                std::map<std::string, Instruction *> &instructionMap, bool backward) {
    std::string instructionName = getInstructionString(I);
    Value *op1 = I.getOperand(0);
    Value *op2 = I.getOperand(1);

    if (!backward) {
        if (isa<llvm::ConstantInt>(op2)) {
            auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
            std::pair<int, int> interval_op1 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))]];
            blockMap[instructionMap[instructionName]] = subOp(interval_op1, std::make_pair(op2_val, op2_val));
        } else if (isa<llvm::ConstantInt>(op1)) {
            auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
            std::pair<int, int> interval_op2 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op2))]];
            blockMap[instructionMap[instructionName]] = subOp(std::make_pair(op1_val, op1_val), interval_op2);
        } else {
            std::pair<int, int> interval_op1 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))]];
            std::pair<int, int> interval_op2 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op2))]];
            blockMap[instructionMap[instructionName]] = subOp(interval_op1, interval_op2);
        }
    } else {
        auto instructionInterval = blockMap[instructionMap[instructionName]];
        if (isa<llvm::ConstantInt>(op2)) {
            auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
            std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
            blockMap[instructionMap[op1_str]] = addOp(instructionInterval, std::make_pair(op2_val, op2_val));
        } else if (isa<llvm::ConstantInt>(op1)) {
            auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
            std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
            blockMap[instructionMap[op2_str]] = subOp(std::make_pair(op1_val, op1_val), instructionInterval);
        } else {
            std::string op1String = getInstructionString(*dyn_cast<Instruction>(op1));
            auto op1Interval = blockMap[instructionMap[op1String]];
            std::string op2String = getInstructionString(*dyn_cast<Instruction>(op2));
            auto op2Interval = blockMap[instructionMap[op2String]];
            std::pair<int, int> op1Temp = subOp(instructionInterval, op2Interval);
            std::pair<int, int> op2Temp = subOp(instructionInterval, op1Interval);
            int newLowerOp1 = std::max(op1Interval.first, op1Temp.first);
            int newUpperOp1 = std::min(op1Interval.second, op1Temp.second);
            int newLowerOp2 = std::max(op2Interval.first, op2Temp.first);
            int newUpperOp2 = std::min(op2Interval.second, op2Temp.second);
            blockMap[instructionMap[op1String]] = std::make_pair(newLowerOp1, newUpperOp1);
            blockMap[instructionMap[op2String]] = std::make_pair(newLowerOp2, newUpperOp2);
        }
    }
}

void analyzeMul(Instruction &I, std::map<Instruction *, std::pair<int, int>> &blockMap,
                std::map<std::string, Instruction *> &instructionMap, bool backward) {
    std::string instructionName = getInstructionString(I);
    Value *op1 = I.getOperand(0);
    Value *op2 = I.getOperand(1);

    if (!backward) {
        if (isa<llvm::ConstantInt>(op2)) {
            auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
            std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
            std::pair<int, int> interval_op1 = blockMap[instructionMap[op1_str]];
            blockMap[instructionMap[instructionName]] = mulOp(interval_op1, std::pair<int, int>(op2_val, op2_val));
        } else if (isa<llvm::ConstantInt>(op1)) {
            auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
            std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
            std::pair<int, int> interval_op2 = blockMap[instructionMap[op2_str]];
            blockMap[instructionMap[instructionName]] = mulOp(interval_op2, std::pair<int, int>(op1_val, op1_val));
        } else {
            std::pair<int, int> interval_op1 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))]];
            std::pair<int, int> interval_op2 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op2))]];
            blockMap[instructionMap[instructionName]] = mulOp(interval_op1, interval_op2);
        }
    } else {
        auto instructionInterval = blockMap[instructionMap[instructionName]];
        if (isa<llvm::ConstantInt>(op2)) {
            auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
            std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
            blockMap[instructionMap[op1_str]] = divOp(instructionInterval, std::pair<int, int>(op2_val, op2_val));
        } else if (isa<llvm::ConstantInt>(op1)) {
            auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
            std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
            blockMap[instructionMap[op2_str]] = divOp(instructionInterval, std::pair<int, int>(op1_val, op1_val));
        } else {
            std::string op1String = getInstructionString(*dyn_cast<Instruction>(op1));
            auto op1Interval = blockMap[instructionMap[op1String]];
            std::string op2String = getInstructionString(*dyn_cast<Instruction>(op2));
            auto op2Interval = blockMap[instructionMap[op2String]];
            std::pair<int, int> op1Temp = divOp(instructionInterval, op2Interval);
            std::pair<int, int> op2Temp = divOp(instructionInterval, op1Interval);
            int newLowerOp1 = std::max(op1Interval.first, op1Temp.first);
            int newUpperOp1 = std::min(op2Interval.second, op1Temp.second);
            int newLowerOp2 = std::max(op2Interval.first, op2Temp.first);
            int newUpperOp2 = std::min(op2Interval.second, op2Temp.second);
            blockMap[instructionMap[op1String]] = std::make_pair(newLowerOp1, newUpperOp1);
            blockMap[instructionMap[op2String]] = std::make_pair(newLowerOp2, newUpperOp2);
        }
    }
}

void analyzeSrem(Instruction &I, std::map<Instruction *, std::pair<int, int>> &blockMap,
                 std::map<std::string, Instruction *> &instructionMap, bool backward) {
    std::string instructionName = getInstructionString(I);
    Value *op1 = I.getOperand(0);
    Value *op2 = I.getOperand(1);

    if (!backward) {
        if (isa<llvm::ConstantInt>(op2)) {
            auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
            std::pair<int, int> interval_op1 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))]];
            blockMap[instructionMap[instructionName]] = sremOp(interval_op1, std::make_pair(op2_val, op2_val));
        } else if (isa<llvm::ConstantInt>(op1)) {
            auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
            std::pair<int, int> interval_op2 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op2))]];
            blockMap[instructionMap[instructionName]] = sremOp(std::make_pair(op1_val, op1_val), interval_op2);
        } else {
            std::pair<int, int> interval_op1 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))]];
            std::pair<int, int> interval_op2 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op2))]];
            blockMap[instructionMap[instructionName]] = sremOp(interval_op1, interval_op2);
        }
    } else {
        auto instructionInterval = blockMap[instructionMap[instructionName]];
        if (isa<llvm::ConstantInt>(op2)) {
            std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
        } else if (isa<llvm::ConstantInt>(op1)) {
            std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
        } else {
            std::string op1String = getInstructionString(*dyn_cast<Instruction>(op1));
            std::string op2String = getInstructionString(*dyn_cast<Instruction>(op2));
        }
    }
}

void analyzeStore(Instruction &I, std::map<Instruction *, std::pair<int, int>> &blockMap,
                  std::map<std::string, Instruction *> &instructionMap, bool backward) {
    Value *op1 = I.getOperand(0);
    Value *op2 = I.getOperand(1);
    if (!backward) {
        if (isa<llvm::ConstantInt>(op1)) {
            auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
            std::string op2Str = getInstructionString(*dyn_cast<Instruction>(op2));
            blockMap[instructionMap[op2Str]] = std::make_pair(op1_val, op1_val);
        } else {
            std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
            std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
            blockMap[instructionMap[op2_str]] = blockMap[instructionMap[op1_str]];
        }
    } else {
        if (!isa<llvm::ConstantInt>(op1)) {
            std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
            std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
            blockMap[instructionMap[op1_str]] = blockMap[instructionMap[op2_str]];
        }
    }
}

void analyzeLoad(Instruction &I, std::map<Instruction *, std::pair<int, int>> &blockMap,
                 std::map<std::string, Instruction *> &instructionMap, bool backward) {
    if (!backward) {
        auto *op_instruction = dyn_cast<Instruction>(I.getOperand(0));
        std::string str = getInstructionString(*op_instruction);
        blockMap[instructionMap[getInstructionString(I)]] = blockMap[instructionMap[str]];
    } else {
        auto *op_instruction = dyn_cast<Instruction>(I.getOperand(0));
        std::string str = getInstructionString(*op_instruction);
        if (checkEmpty(blockMap[instructionMap[str]])) { ;
        } else if (checkEmpty(blockMap[instructionMap[getInstructionString(I)]])) {
            blockMap[instructionMap[str]].first = limitRange(POS_LIMIT);
            blockMap[instructionMap[str]].second = limitRange(NEG_LIMIT);
        } else {
            int lower = std::max(blockMap[instructionMap[str]].first,
                                 blockMap[instructionMap[getInstructionString(I)]].first);
            int upper = std::min(blockMap[instructionMap[str]].second,
                                 blockMap[instructionMap[getInstructionString(I)]].second);
            blockMap[instructionMap[str]].first = limitRange(lower);
            blockMap[instructionMap[str]].second = limitRange(upper);
        }
    }
}

void analyzeAlloca(Instruction &I, std::map<Instruction *, std::pair<int, int>> &blockMap,
                   std::map<std::string, Instruction *> &instructionMap, bool backward) {
    if (!backward) {
        blockMap[instructionMap[getInstructionString(I)]] = std::make_pair(NEG_LIMIT, POS_LIMIT);
    } else {
        return;
    }
}

//------COMPARISON FUNCTION IMPLEMENTATION
void comp(std::pair<int, int> &intervalX, std::pair<int, int> &intervalY, std::pair<int, int> &intervalR) {
    std::pair<int, int> xRange = addOp(intervalY, intervalR);
    intervalX.first = limitRange(std::max(xRange.first, intervalX.first));
    intervalX.second = limitRange(std::min(xRange.second, intervalX.second));
    if (intervalX.first > intervalX.second) {
        intervalX.first = limitRange(POS_LIMIT);
        intervalX.second = limitRange(NEG_LIMIT);
    }
    std::pair<int, int> yMinusRange = addOp(intervalR, intervalX);
    intervalY.first = limitRange(std::max(yMinusRange.second * -1, intervalY.first));
    intervalY.second = limitRange(std::min(yMinusRange.first * -1, intervalY.second));
    if (intervalY.first > intervalY.second) {
        intervalY.first = limitRange(POS_LIMIT);
        intervalY.second = limitRange(NEG_LIMIT);
    }
}

void compare(Value *cmpOp1, Value *cmpOp2, std::pair<int, int> &R, std::map<Instruction *, std::pair<int, int>> &result,
             std::map<std::string, Instruction *> &instructionMap) {
    if (isa<llvm::ConstantInt>(cmpOp1)) {
        auto op1_val = dyn_cast<llvm::ConstantInt>(cmpOp1)->getZExtValue();
        std::pair<int, int> X(op1_val, op1_val);
        comp(X, result[instructionMap[getInstructionString(*dyn_cast<Instruction>(cmpOp2))]], R);
    } else if (isa<llvm::ConstantInt>(cmpOp2)) {
        auto op2_val = dyn_cast<llvm::ConstantInt>(cmpOp2)->getZExtValue();
        std::pair<int, int> Y(op2_val, op2_val);
        comp(result[instructionMap[getInstructionString(*dyn_cast<Instruction>(cmpOp1))]], Y, R);
    } else {
        comp(result[instructionMap[getInstructionString(*dyn_cast<Instruction>(cmpOp1))]],
             result[instructionMap[getInstructionString(*dyn_cast<Instruction>(cmpOp2))]], R);
    }
}

void analyzeBr(BasicBlock *BB, Instruction &I, std::map<Instruction *, std::pair<int, int>> &blockMap,
               std::map<std::string, Instruction *> &instructionMap,
               std::map<BasicBlock *, std::map<Instruction *, std::pair<int, int>>> &result) {
    auto *branchInst = dyn_cast<BranchInst>(&I);
    auto *bb_op1 = branchInst->getSuccessor(0);
    if (!(branchInst->isConditional())) {
        result[bb_op1] = blockMap;
        return;
    }
    //cmp instruction
    auto *cmpIns = dyn_cast<ICmpInst>(I.getOperand(0));
    //cmp variables
    Value *cmpOp1 = cmpIns->getOperand(0);
    Value *cmpOp2 = cmpIns->getOperand(1);
    //br blocks
    auto *bb_op2 = dyn_cast<BasicBlock>(branchInst->getSuccessor(1));

    std::map<Instruction *, std::pair<int, int>> branch1 = blockMap;
    std::map<Instruction *, std::pair<int, int>> branch2 = blockMap;

    std::pair<int, int> Y = std::make_pair(NEG_LIMIT, POS_LIMIT);
    switch (cmpIns->getSignedPredicate()) {
        case CmpInst::Predicate::ICMP_SGT : {
            std::pair<int, int> R1 = std::make_pair(1, POS_LIMIT);
            std::pair<int, int> R2 = std::make_pair(NEG_LIMIT, 0);
            compare(cmpOp1, cmpOp2, R1, branch1, instructionMap);
            compare(cmpOp1, cmpOp2, R2, branch2, instructionMap);
            break;
        }
        case CmpInst::Predicate::ICMP_SGE : {
            std::pair<int, int> R1 = std::make_pair(0, POS_LIMIT);
            std::pair<int, int> R2 = std::make_pair(NEG_LIMIT, -1);
            compare(cmpOp1, cmpOp2, R1, branch1, instructionMap);
            compare(cmpOp1, cmpOp2, R2, branch2, instructionMap);
            break;
        }
        case CmpInst::Predicate::ICMP_SLT : {
            std::pair<int, int> R1 = std::make_pair(NEG_LIMIT, -1);
            std::pair<int, int> R2 = std::make_pair(0, POS_LIMIT);
            compare(cmpOp1, cmpOp2, R1, branch1, instructionMap);
            compare(cmpOp1, cmpOp2, R2, branch2, instructionMap);
            break;
        }
        case CmpInst::Predicate::ICMP_SLE : {
            std::pair<int, int> R1 = std::make_pair(NEG_LIMIT, 0);
            std::pair<int, int> R2 = std::make_pair(1, POS_LIMIT);
            compare(cmpOp1, cmpOp2, R1, branch1, instructionMap);
            compare(cmpOp1, cmpOp2, R2, branch2, instructionMap);
            break;
        }
        case CmpInst::Predicate::ICMP_EQ : {
            std::pair<int, int> R1 = std::make_pair(0, 0);
            std::pair<int, int> R2 = std::make_pair(NEG_LIMIT, POS_LIMIT);
            compare(cmpOp1, cmpOp2, R1, branch1, instructionMap);
            compare(cmpOp1, cmpOp2, R2, branch2, instructionMap);
            break;
        }
        case CmpInst::Predicate::ICMP_NE : {
            std::pair<int, int> R1 = std::make_pair(NEG_LIMIT, POS_LIMIT);
            std::pair<int, int> R2 = std::make_pair(0, 0);
            compare(cmpOp1, cmpOp2, R1, branch1, instructionMap);
            compare(cmpOp1, cmpOp2, R2, branch2, instructionMap);
            break;
        }
        default: {
            std::cout << "Alert: " << "Compare Type Unknown." << std::endl;
            break;
        }
    }
    // if (debug) {
    //     std::cout << ">>>>AFTER COMPARE:" << std::endl;
    //     std::cout << ">> BRANCH1" << std::endl;
    //     for (auto &p : branch1) {
    //         std::cout << getInstructionString(*p.first) << " >> " << p.second.getIntervalString() << std::endl;
    //     }
    //     std::cout << ">> BRANCH2" << std::endl;
    //     for (auto &p : branch2) {
    //         std::cout << getInstructionString(*p.first) << " >> " << p.second.getIntervalString() << std::endl;
    //     }
    // }
    auto it = BB->rend();
    bool conti = true;
    while (conti) {
        it--;
        if (it == BB->rbegin()) {
            conti = false;
        }
        switch (it->getOpcode()) {
            case Instruction::Add: {
                analyzeAdd(*it, branch1, instructionMap, true);
                analyzeAdd(*it, branch2, instructionMap, true);
            }
            case Instruction::Sub: {
                analyzeSub(*it, branch1, instructionMap, true);
                analyzeSub(*it, branch2, instructionMap, true);
                break;
            }
            case Instruction::Mul: {
                analyzeMul(*it, branch1, instructionMap, true);
                analyzeMul(*it, branch2, instructionMap, true);
                break;
            }
            case Instruction::SRem: {
                analyzeSrem(*it, branch1, instructionMap, true);
                analyzeSrem(*it, branch2, instructionMap, true);
                break;
            }
            case Instruction::Alloca: {
                analyzeAlloca(*it, branch1, instructionMap, true);
                analyzeAlloca(*it, branch2, instructionMap, true);
                break;
            }
            case Instruction::Store: {
                analyzeStore(*it, branch1, instructionMap, true);
                analyzeStore(*it, branch2, instructionMap, true);
                break;
            }
            case Instruction::Load: {
                analyzeLoad(*it, branch1, instructionMap, true);
                analyzeLoad(*it, branch2, instructionMap, true);
                break;
            }
            default: {
                break;
            }
        }

    }
    for (auto it = BB->rend(); it != BB->rbegin(); it--) {

    }
    result[bb_op1] = branch1;
    result[bb_op2] = branch2;
}

