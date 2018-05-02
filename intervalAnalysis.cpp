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

std::map<BasicBlock *, std::vector<std::map<Instruction *, std::pair<int, int>>>> varIntervalMap;

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

std::pair<int, int> intersection(std::pair<int, int> interval1, std::pair<int, int> interval2) {
        // if ((interval1.first == POS_LIMIT || interval2.first == POS_LIMIT) || (interval1.first > interval2.second || interval2.first > interval1.second)) {
        //     return std::make_pair(POS_LIMIT, NEG_LIMIT);
        // } else {
        //     return std::make_pair(std::max(interval1.first, interval2.first), std::min(interval1.second, interval2.second));
        // }

        if (interval1.first == POS_LIMIT || interval2.first == POS_LIMIT) {
            return std::make_pair(POS_LIMIT, NEG_LIMIT);
        } else if (interval1.first > interval2.second || interval2.first > interval1.second) {
            return std::make_pair(POS_LIMIT, NEG_LIMIT);
        } else {
            return std::make_pair(std::max(interval1.first, interval2.first), std::min(interval1.second, interval2.second));
        }
}

void allocaOperation(Instruction &I, std::map<Instruction *, std::pair<int, int>> &instrRangeMap, std::map<std::string, Instruction *> &instrMap);
void storeOperation(Instruction &I, std::map<Instruction *, std::pair<int, int>> &instrRangeMap, std::map<std::string, Instruction *> &instrMap);
void loadOperation(Instruction &I, std::map<Instruction *, std::pair<int, int>> &instrRangeMap, std::map<std::string, Instruction *> &instrMap);
void mathOperation(Instruction &I, std::map<Instruction *, std::pair<int, int>> &instrRangeMap, std::map<std::string, Instruction *> &instrMap);
void branchOperation(BasicBlock *BB, Instruction &I, std::map<Instruction *, std::pair<int, int>> &instrRangeMap, std::map<std::string, Instruction *> &instrMap, std::map<BasicBlock *, std::map<Instruction *, std::pair<int, int>>> &updatedBlockMap);
bool blockAnalysis(BasicBlock *BB, std::map<Instruction *, std::pair<int, int>> &instrRangeMap, std::map<std::string, Instruction *> &instrMap,
                  std::map<BasicBlock *, std::map<Instruction *, std::pair<int, int>>> &analysisMap, std::map<BasicBlock *, std::map<Instruction *, std::pair<int, int>>> &updatedBlockMap);
bool analyzeBlockWithContext(BasicBlock *BB,
                             std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>> &input,
                             std::map<std::string, Instruction *> &instructionMap,
                             std::map<BasicBlock *, std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>>> &contextAnalysisMap,
                             std::map<BasicBlock *, std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>>> &result);

// void printBasicBlockContext(std::map<BasicBlock *, std::vector<std::map<Instruction *, std::pair<int, int>>>> context) {
//     for (auto &p : context) {
//         std::cout << getBasicBlockLabel(p.first) << std::endl;
//         for (auto &pp : p.second) {
//             std::cout << "Case:" << std::endl;
//             for (auto &ppp : pp) {
//                 std::cout << getInstructionString(*(ppp.first)) << "  >>  " << ppp.second.getIntervalString()
//                           << std::endl;
//             }
//         }
//     }
// }

// void printcontextCombination(std::vector<std::map<Instruction *, std::pair<int, int>>> &contextCombination) {
//     for (auto &combination : contextCombination) {
//         std::cout << "================Context=============" << std::endl;
//         for (auto &combination_pair : combination) {
//             std::cout << getInstructionString(*(combination_pair.first)) << "  >>  "
//                       << combination_pair.second.getIntervalString()
//                       << std::endl;
//         }
//     }
// }


std::vector<std::pair<int,int>> getOperandIntervals(Instruction &I, Value *op1, Value *op2, std::map<Instruction *, std::pair<int,int>> context,
                                             std::map<Instruction *, std::pair<int,int>> &variables, std::map<std::string, Instruction *> &instructionMap) {
    bool isInContext = context.find(&I) != context.end();
    std::pair<int,int> interval_op1;
    std::pair<int,int> interval_op2;
    if (isa<llvm::ConstantInt>(op2)) {
        auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
        auto op1_instr = instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))];
        interval_op1 = variables[op1_instr];
        interval_op2 = std::make_pair(op2_val, op2_val);

    } else if (isa<llvm::ConstantInt>(op1)) {
        auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
        auto op2_instr = instructionMap[getInstructionString(*dyn_cast<Instruction>(op2))];
        interval_op1 = std::make_pair(op1_val, op1_val);
        interval_op2 = variables[op2_instr];
    } else {
        auto op1_instr = instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))];
        auto op2_instr = instructionMap[getInstructionString(*dyn_cast<Instruction>(op2))];
        interval_op1 = variables[op1_instr];
        interval_op2 = variables[op2_instr];
    }
    std::vector<std::pair<int,int>> results;
    results.push_back(interval_op1);
    results.push_back(interval_op2);
    return results;
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

            std::vector<Instruction *> intersetInstructionSet;
            sort(intersetInstructionSet.begin(), intersetInstructionSet.end());
            sort(tempSet.begin(), tempSet.end());
            set_intersection(intersetInstructionSet.begin(), intersetInstructionSet.end(), tempSet.begin(), tempSet.end(), back_inserter(intersetInstructionSet));

            // intersetInstructionSet = intersection(intersetInstructionSet, tempSet);
        }
    }
    for (auto it = blockMap.begin(); it != blockMap.end(); ++it) {
        if (it == blockMap.begin()) {
            for (auto &element : intersetInstructionSet) {
                    unionSet.insert(std::make_pair(element, it->second[element]));

            }
        } else {
            for (auto &element : intersetInstructionSet) {

                unionSet[element] = std::make_pair(std::min(unionSet[element].first, it->second[element].first), std::max(unionSet[element].second, it->second[element].second));
                    // unionSet[element] = varInteral::getUnion(unionSet[element], it->second[element]);
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

void cleanUpEmpty(BasicBlock *bb, std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>> &blockMap) {
    for (auto &pair : blockMap) {
        bool containsEmpty = false;

        for (auto &c : *pair.first) {
            pair.second[c.first] = intersection(c.second, pair.second[c.first]);
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
std::string getBasicBlockLabel(BasicBlock *BB) {
    std::string basicBlockStr;
    llvm::raw_string_ostream rso(basicBlockStr);
    rso << ">>>>Basic Block: ";
    BB->printAsOperand(rso, false);
    return basicBlockStr;
}

void printAnalysisMap(std::map<BasicBlock *, std::map<Instruction *, std::pair<int, int>>> &analysisMap,
                      std::map<std::string, Instruction *> &instructionMap) {
    std::cout << "==============Analysis Report==============" << std::endl;
    for (auto &m : analysisMap) {
        std::cout << getBasicBlockLabel(m.first) << std::endl;
        for (auto &_m : m.second) {
            std::string intervalString;
            if (checkEmpty(_m.second))
                intervalString = "EMPTY SET";
            else 
                intervalString = (_m.second.first == NEG_LIMIT ? "NEG_LIMIT" : std::to_string(_m.second.first)) + "->" +
                   (_m.second.second == POS_LIMIT ? "POS_LIMIT" : std::to_string(
                           _m.second.second));
            std::cout << getInstructionString(*(_m.first)) << "  >>  " << intervalString << std::endl;
        }
        for (auto mm = m.second.begin(); mm != m.second.end(); mm++) {
            std::string temp_mm = (*mm).first->getName().str().c_str();
            if (temp_mm.size() == 0) {
                continue;
            }
            for (auto mm1 = mm; mm1 != m.second.end(); mm1++) {
                std::string temp_mm1 = (*mm1).first->getName().str().c_str();
                if (temp_mm1.size() == 0 || temp_mm == temp_mm1) {
                    continue;
                }
                int v1_range[2] = {(*mm).second.first, (*mm).second.second};
                int v2_range[2] = {(*mm1).second.first, (*mm1).second.second};
                if (v1_range[0] == NEG_LIMIT || v1_range[1] == POS_LIMIT) {
                    std::cout << temp_mm << " <--> " << temp_mm1 << " : "
                              << "INF" << std::endl;
                } else if (v2_range[0] == NEG_LIMIT || v2_range[1] == POS_LIMIT) {
                    std::cout << temp_mm << " <--> " << temp_mm1 << " : "
                              << "INF" << std::endl;
                } else {
                    int a = std::abs(v1_range[0] - v2_range[1]);
                    int b = std::abs(v1_range[1] - v2_range[0]);
                    std::cout << temp_mm << " <--> " << temp_mm1 << " : "
                              << (a > b ? (a >= POS_LIMIT ? "INF" : std::to_string(a)) : (b >=
                                                                                                     POS_LIMIT
                                                                                                     ? "INF"
                                                                                                     : std::to_string(
                                              b)))
                              << std::endl;
                }

            }
        }
        std::cout << "===========================================" << std::endl;
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
            if(pair.first->getName().size()!=0){
                std::string intervalString;
                if (checkEmpty(pair.second)){
                    intervalString = "EMPTY SET";
                }
                else {
                    intervalString = (pair.second.first == NEG_LIMIT ? "NEG_LIMIT" : std::to_string(pair.second.first)) + "->" +
                       (pair.second.second == POS_LIMIT ? "POS_LIMIT" : std::to_string(
                               pair.second.second));
                }
                std::cout << getInstructionString(*pair.first) << "  -> " << intervalString
                      << std::endl;
            }
        }
    }
}

void printBasicBlockContext(std::map<BasicBlock *, std::vector<std::map<Instruction *, std::pair<int, int>>>> context) {
    for (auto &p : context) {
        std::cout << getBasicBlockLabel(p.first) << std::endl;
        for (auto &pp : p.second) {
            std::cout << "Case:" << std::endl;
            for (auto &ppp : pp) {
                std::string intervalString;
                if (checkEmpty(ppp.second))
                    intervalString = "EMPTY SET";
                else 
                    intervalString = (ppp.second.first == NEG_LIMIT ? "NEG_LIMIT" : std::to_string(ppp.second.first)) + "->" +
                       (ppp.second.second == POS_LIMIT ? "POS_LIMIT" : std::to_string(
                               ppp.second.second));
                std::cout << getInstructionString(*(ppp.first)) << "  >>  " << intervalString
                          << std::endl;
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

//main function
int main(int argc, char **argv) 
{
    // Read the IR file.
    LLVMContext &Context = getGlobalContext();
    SMDiagnostic Err;

    // Extract Module M from IR (assuming only one Module exists)
    Module *M = ParseIRFile(argv[1], Err, Context);
    if (M == nullptr)
    {
      fprintf(stderr, "error: failed to load LLVM IR file \"%s\"", argv[1]);
      return EXIT_FAILURE;
    }

    Function *F = M->getFunction("main");

    BasicBlock *entryBB = &F->getEntryBlock();

    //variable ranges of all basic blocks
    std::map<std::string, Instruction *> varMap;
    std::map<BasicBlock *, std::map<Instruction *, std::pair<int, int>>> analysisMap;

    std::stack<std::pair<BasicBlock *, std::map<Instruction *, std::pair<int, int>>>> traversalStack;

    std::vector<std::map<Instruction *, std::pair<int, int>>> globalVarIntervalMap;

    std::map<Instruction *, std::pair<int, int>> emptySet;
    traversalStack.push(std::make_pair(entryBB, emptySet));

    int count = 0;
    while (!traversalStack.empty() && count < 10000) {
        std::map<BasicBlock *, std::map<Instruction *, std::pair<int, int>>> finalMap;
        auto bMap = traversalStack.top();
        traversalStack.pop();

        bool updated = blockAnalysis(bMap.first, bMap.second, varMap, analysisMap, finalMap);

        if (updated) {
            for (auto &p : finalMap) {
                traversalStack.push(p);
            }
        }
        count++;
    }

    // printAnalysisMap(analysisMap, varMap);

    for(auto &row : analysisMap) {

        for (auto &interval : row.second) {
            interval.second = std::make_pair(NEG_LIMIT, POS_LIMIT);
        }

        BasicBlock *BB = row.first;
        std::map<Instruction *, std::pair<int, int>> instrRangeMap = row.second;
        std::map<BasicBlock *, std::map<Instruction *, std::pair<int, int>>> updatedBlockMap;

        for(auto &instr : *BB) {
            if(instr.getOpcode() == Instruction::Br) {

                branchOperation(BB, instr, instrRangeMap, varMap, updatedBlockMap);

                if (updatedBlockMap.size() <= 1)
                    break;
                
                    for(auto &instrMap : updatedBlockMap){
                        varIntervalMap[BB].push_back(instrMap.second);
                    }
                    for (auto iter = varIntervalMap[BB][0].begin(); iter != varIntervalMap[BB][0].end();) {
                        bool match = false;
                        for (auto iter1 = varIntervalMap[BB][1].begin(); iter1 != varIntervalMap[BB][1].end();) {
                            std::string intervalString1;
                            std::string intervalString2;
                            if (checkEmpty(iter->second)){
                                intervalString1 = "EMPTY";
                            } else {
                                intervalString1 = (iter->second.first == NEG_LIMIT ? "NEG_LIMIT" : std::to_string(iter->second.first)) + "-->" +
                                   (iter->second.second == POS_LIMIT ? "POS_LIMIT" : std::to_string(iter->second.second));
                            }
                            if (checkEmpty(iter1->second)){
                                intervalString2 = "EMPTY";
                            } else {
                                intervalString2 = (iter1->second.first == NEG_LIMIT ? "NEG_LIMIT" : std::to_string(iter1->second.first)) + "-->" +
                                   (iter1->second.second == POS_LIMIT ? "POS_LIMIT" : std::to_string(iter1->second.second));
                            }
                            if(intervalString1 == intervalString2)
                            {
                                iter = varIntervalMap[BB][0].erase(iter);
                                iter1 = varIntervalMap[BB][1].erase(iter1);
                                match = true;
                                break;
                            }
                            iter1++;
                        }
                        if (!match)
                            iter++;
                    }
                    for(auto iter = varIntervalMap[BB][0].begin(); iter != varIntervalMap[BB][0].end();){
                        if(iter->first->getName().size() == 0){
                            iter = varIntervalMap[BB][0].erase(iter);
                        }else{
                            iter++;
                        }
                    }
                    for(auto iter = varIntervalMap[BB][1].begin(); iter != varIntervalMap[BB][1].end();){
                        if(iter->first->getName().size() == 0){
                            iter = varIntervalMap[BB][1].erase(iter);
                        }else{
                            iter++;
                        }
                    }
            }
        }
    }

    // printBasicBlockContext(varIntervalMap);

    // varInterval: <BasicBlock *, std::vector<std::map<Instruction *, std::pair<int, int>>>>
    for(auto varInterval : varIntervalMap) {
            if(varInterval.second.size() == 0 || varInterval.second[0].size() == 0) {
                continue;
            }
            if(globalVarIntervalMap.size() == 0) {
                globalVarIntervalMap = varInterval.second;
            } else {
                std::vector<std::map<Instruction *, std::pair<int, int>>> copy = globalVarIntervalMap;
                globalVarIntervalMap.clear();
                // instrMap: std::vector<std::map<Instruction *, std::pair<int, int>>>
                for(auto instrMap : varInterval.second) {
                    // globalInstrMap: std::map<Instruction *, std::pair<int, int>>
                    for(auto globalInstrMap : copy) {
                        // iter: std::map<Instruction *, std::pair<int, int>>
                        for(auto iter = instrMap.begin(); iter != instrMap.end(); iter++) {
                            if (globalInstrMap.find(iter->first) == globalInstrMap.end()) {
                                globalInstrMap.insert(*iter);
                            } else {
                                std::pair<int, int> intersectionResult = intersection(globalInstrMap.find(iter->first)->second, iter->second);
                                globalInstrMap.insert(std::make_pair(iter->first,intersectionResult));
                            }
                        }
                        globalVarIntervalMap.push_back(globalInstrMap);
                    }
                }
            }
    }

    for (auto &combination : globalVarIntervalMap) {
        std::cout << "================Context=============" << std::endl;
        for (auto &combination_pair : combination) {
            std::string str;
            if (checkEmpty(combination_pair.second))
                str = "EMPTY SET";
            str = (combination_pair.second.first == NEG_LIMIT ? "NEG_LIMIT" : std::to_string(combination_pair.second.first)) + "-" +
                   (combination_pair.second.second == POS_LIMIT ? "POS_LIMIT" : std::to_string(
                           combination_pair.second.second));
            std::cout << getInstructionString(*(combination_pair.first)) << "  >>  "
                      << str
                      << std::endl;
        }
    }

    std::map<BasicBlock *, std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>>> intervalAnalysisMap;
    std::stack<std::pair<BasicBlock *, std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>>>> intervalTraversalStack;

    runBlockIntervalAnalysis(entryBB, globalVarIntervalMap, varMap, intervalAnalysisMap, intervalTraversalStack);

    printAnalysisMapWithContext(intervalAnalysisMap);
}

bool checkUpdates(std::map<Instruction *, std::pair<int, int>> &instrRangeMap, std::map<Instruction *, std::pair<int, int>> &analysisMap) 
{
    bool changed = false;
    for (auto &p : instrRangeMap) {
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
        }
    }
    return changed;
}

bool intervalCheckUpdates(std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>> &blockIntervalMap,
                          std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>> &updatedAnalysisMap) {
    bool changed = false;
    for (auto &intervalMap : blockIntervalMap) {
        
        // varInterval: std::map<Instruction *, std::pair<int, int>>
        auto varIntervalResult = updatedAnalysisMap[intervalMap.first];
        // intervalMap: std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>
        auto varInterval = intervalMap.second;

        for (auto &varRange : varInterval) {
            if (varIntervalResult.find(varRange.first) == varIntervalResult.end()) {
                updatedAnalysisMap[intervalMap.first][varRange.first] = varRange.second;
                changed = true;
            } else if (checkEmpty(varIntervalResult[varRange.first])) {
                if (!checkEmpty(varRange.second)) {
                    updatedAnalysisMap[intervalMap.first][varRange.first] = varRange.second;
                    changed = true;
                }
            } else if (!(varRange.second <= varIntervalResult[varRange.first])) {
                updatedAnalysisMap[intervalMap.first][varRange.first].first = limitRange(std::min(varRange.second.first, varIntervalResult[varRange.first].first));
                updatedAnalysisMap[intervalMap.first][varRange.first].second = limitRange(std::max(varRange.second.second, varIntervalResult[varRange.first].second));
                changed = true;
            }
        }
    }
    return changed;
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

void operatorWithContext(BasicBlock *bb, Instruction &I,
                         std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>> &blockMap,
                         std::map<std::string, Instruction *> &instructionMap, std::string op) {
    std::cout << getInstructionString(I) << std::endl;
    for(auto &pair : blockMap) {
        std::cout << "=======================bloack start======================" <<std::endl;
        for(auto &i : pair.second) {
            std::cout << getInstructionString(*(i.first)) << std::endl;
            std::cout << i.second.first << "-->" << i.second.second << std::endl;
        }
        std::cout << "========================block end========================" <<std::endl;
    }

    if (blockMap.begin()->first->find(&I) != blockMap.begin()->first->end()) {
        auto join = joinWithContext(bb, blockMap);
        for (auto &pair :blockMap) {
            auto context = *pair.first;
            auto result = getOperandIntervals(I, I.getOperand(0), I.getOperand(1), *pair.first, join, instructionMap);
            std::pair<int, int> temp = getOp(result[0], result[1], op);
            if (checkEmpty(intersection(temp, context[&I])))
                for (auto &var : pair.second)
                    var.second = std::make_pair(POS_LIMIT, NEG_LIMIT);
            else {
                pair.second = join;
                pair.second[&I] = intersection(temp, context[&I]);
            }
        }
    } else {
        std::cout << "enter else" << std::endl;
        for (auto &pair : blockMap) {
            auto context = *pair.first;
            auto result = getOperandIntervals(I, I.getOperand(0), I.getOperand(1), *pair.first, pair.second,
                                              instructionMap);
            std::pair<int, int> temp = getOp(result[0], result[1], op);
            std::cout << getInstructionString(I) << std::endl;
            pair.second[&I] = temp;
        }
         std::cout << "exit else" << std::endl;
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
            if (checkEmpty(intersection(temp, context[op2_instr]))) {
                for (auto &var : pair.second)
                    var.second = std::make_pair(POS_LIMIT, NEG_LIMIT);
            } else {
                pair.second = join;
                pair.second[op2_instr] = intersection(temp, context[op2_instr]);
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
            if (checkEmpty(intersection(temp, context[&I]))) {
                for (auto &var : pair.second)
                    var.second = std::pair<int, int>(POS_LIMIT, NEG_LIMIT);
            } else {
                pair.second = join;
                pair.second[&I] = intersection(temp, context[&I]);
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


    auto branch_1_context = varIntervalMap[BB][0];
    auto branch_2_context = varIntervalMap[BB][1];


    for (auto &pair : total_branch1) {
        bool emptyBranch1 = false;
        for (auto &context_pair : *pair.first) {
            if (branch_1_context.find(context_pair.first) != branch_1_context.end() &&
                checkEmpty(intersection(branch_1_context[context_pair.first], context_pair.second))) {
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
                checkEmpty(intersection(branch_2_context[context_pair.first], context_pair.second))) {
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


bool belongTo(std::pair<int, int> v1, std::pair<int, int> v2) {
    if (v1.first == POS_LIMIT && v1.second == NEG_LIMIT) {
        return true;
    }
    if (v2.first == POS_LIMIT && v2.second == NEG_LIMIT) {
        return false;
    }
    return v1.first >= v2.first && v1.second <= v2.second;
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
            } else if (!(belongTo(p.second , analysisMap_result[p.first]))) {
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

bool blockAnalysis(BasicBlock *BB, std::map<Instruction *, std::pair<int, int>> &instrRangeMap, std::map<std::string, Instruction *> &instrMap,
                  std::map<BasicBlock *, std::map<Instruction *, std::pair<int, int>>> &analysisMap,
                  std::map<BasicBlock *, std::map<Instruction *, std::pair<int, int>>> &updatedBlockMap) {
    for (auto &I: *BB) {
        std::string instructionStr = getInstructionString(I);
        instrMap[instructionStr] = &I;
        switch (I.getOpcode()) {
            case Instruction::Alloca: {
                allocaOperation(I, instrRangeMap, instrMap);
                break;
            }
            case Instruction::Store: {
                storeOperation(I, instrRangeMap, instrMap);
                break;
            }
            case Instruction::Load: {
                loadOperation(I, instrRangeMap, instrMap);
                break;
            }
            case Instruction::Add: 
            case Instruction::Sub: 
            case Instruction::Mul: 
            case Instruction::SRem: {
                mathOperation(I, instrRangeMap, instrMap);
                break;
            }
            case Instruction::Br: {
                branchOperation(BB, I, instrRangeMap,instrMap, updatedBlockMap);
                return checkUpdates(instrRangeMap, analysisMap[BB]);
            }
            case Instruction::Ret: {
                return checkUpdates(instrRangeMap, analysisMap[BB]);
            }
            default: {
                break;
            }
        }
    }
    return false;
}

void mathOperation(Instruction &I, std::map<Instruction *, std::pair<int, int>> &instrRangeMap, std::map<std::string, Instruction *> &instrMap) {
    std::string instructionName = getInstructionString(I);
    std::pair<int, int> varRange1, varRange2;
    if(!isa<llvm::ConstantInt>(I.getOperand(0)) && !isa<llvm::ConstantInt>(I.getOperand(1))) {
        varRange1 = instrRangeMap[instrMap[getInstructionString(*dyn_cast<Instruction>(I.getOperand(0)))]];
        varRange2 = instrRangeMap[instrMap[getInstructionString(*dyn_cast<Instruction>(I.getOperand(1)))]];
    } else {
        if(isa<llvm::ConstantInt>(I.getOperand(0))){
            int value = dyn_cast<llvm::ConstantInt>(I.getOperand(0))->getZExtValue();
            std::string varName = getInstructionString(*dyn_cast<Instruction>(I.getOperand(1)));
            std::pair<int, int> range = instrRangeMap[instrMap[varName]];
            varRange1 = std::make_pair(value, value);
            varRange2 = range;
        } else if (isa<llvm::ConstantInt>(I.getOperand(1))){
            int value = dyn_cast<llvm::ConstantInt>(I.getOperand(1))->getZExtValue();
            std::string varName = getInstructionString(*dyn_cast<Instruction>(I.getOperand(0)));
            std::pair<int, int> range = instrRangeMap[instrMap[varName]];
            varRange1 = range;
            varRange2 = std::make_pair(value, value);
        }
    }

    switch(I.getOpcode()){
            case Instruction::Add: {
                instrRangeMap[instrMap[instructionName]] = addOp(varRange1, varRange2);
                break;
            }
            case Instruction::Sub: {
                instrRangeMap[instrMap[instructionName]] = subOp(varRange1, varRange2);
                break;
            }
            case Instruction::Mul: {
                instrRangeMap[instrMap[instructionName]] = mulOp(varRange1, varRange2);
                break;
            }
            case Instruction::SRem: {
                instrRangeMap[instrMap[instructionName]] = sremOp(varRange1, varRange2);
                break;
            }
        }   
}

void backwardAdd(Instruction &I, std::map<Instruction *, std::pair<int, int>> &instrRangeMap, std::map<std::string, Instruction *> &instrMap) {
    std::pair<int, int> originalRange = instrRangeMap[instrMap[getInstructionString(I)]];
    std::pair<int, int> varRange1, varRange2;

    if(!isa<llvm::ConstantInt>(I.getOperand(0)) && !isa<llvm::ConstantInt>(I.getOperand(1))) {
        std::string varName1 = getInstructionString(*dyn_cast<Instruction>(I.getOperand(0)));
        varRange1 = instrRangeMap[instrMap[varName1]];
        std::string varName2 = getInstructionString(*dyn_cast<Instruction>(I.getOperand(1)));
        varRange2 = instrRangeMap[instrMap[varName2]];
        instrRangeMap[instrMap[varName1]] = std::make_pair(std::max(varRange1.first, originalRange.first - varRange2.second), std::min(varRange1.second, originalRange.second - varRange2.first));
        instrRangeMap[instrMap[varName2]] = std::make_pair(std::max(varRange2.first, originalRange.first - varRange1.second), std::min(varRange2.second, originalRange.second - varRange1.first));
    } else {
        std::string varName;
        if(isa<llvm::ConstantInt>(I.getOperand(0))){
            int value = dyn_cast<llvm::ConstantInt>(I.getOperand(0))->getZExtValue();
            varName = getInstructionString(*dyn_cast<Instruction>(I.getOperand(1)));
            varRange2 = std::make_pair(value, value);
        } else if (isa<llvm::ConstantInt>(I.getOperand(1))){
            int value = dyn_cast<llvm::ConstantInt>(I.getOperand(1))->getZExtValue();
            varName = getInstructionString(*dyn_cast<Instruction>(I.getOperand(0)));
            varRange2 = std::make_pair(value, value);
        }
        instrRangeMap[instrMap[varName]] = subOp(originalRange, varRange2);
    }     

    // std::string varString;
    //  if (checkEmpty(instrRangeMap[instrMap[getInstructionString(I)]]))
    //         varString = "EMPTY SET";
    // varString = (instrRangeMap[instrMap[getInstructionString(I)]].first == NEG_LIMIT ? "NEG_LIMIT" : std::to_string(instrRangeMap[instrMap[getInstructionString(I)]].second)) + "-" +
    //            (instrRangeMap[instrMap[getInstructionString(I)]].second == POS_LIMIT ? "POS_LIMIT" : std::to_string(
    //                    instrRangeMap[instrMap[getInstructionString(I)]].second));
    // std::cout << getInstructionString(I) << " ===>" << varString << std::endl;
}

void backwardSub(Instruction &I, std::map<Instruction *, std::pair<int, int>> &instrRangeMap, std::map<std::string, Instruction *> &instrMap) {
    std::pair<int, int> originalRange = instrRangeMap[instrMap[getInstructionString(I)]];
    std::pair<int, int> varRange1, varRange2;

    if(!isa<llvm::ConstantInt>(I.getOperand(0)) && !isa<llvm::ConstantInt>(I.getOperand(1))) {
        std::string varName1 = getInstructionString(*dyn_cast<Instruction>(I.getOperand(0)));
        varRange1 = instrRangeMap[instrMap[varName1]];
        std::string varName2 = getInstructionString(*dyn_cast<Instruction>(I.getOperand(1)));
        varRange2 = instrRangeMap[instrMap[varName2]];
        instrRangeMap[instrMap[varName1]] = std::make_pair(std::max(varRange1.first, subOp(originalRange, varRange2).first), std::min(varRange1.second, subOp(originalRange, varRange2).second));
        instrRangeMap[instrMap[varName2]] = std::make_pair(std::max(varRange2.first, subOp(originalRange, varRange1).first), std::min(varRange2.second, subOp(originalRange, varRange1).second));
    } else {
        std::string varName;
        if(isa<llvm::ConstantInt>(I.getOperand(0))){
            int value = dyn_cast<llvm::ConstantInt>(I.getOperand(0))->getZExtValue();
            varName = getInstructionString(*dyn_cast<Instruction>(I.getOperand(1)));
            varRange2 = std::make_pair(value, value);
            instrRangeMap[instrMap[varName]] = subOp(varRange2, originalRange);
        } else if (isa<llvm::ConstantInt>(I.getOperand(1))){
            int value = dyn_cast<llvm::ConstantInt>(I.getOperand(1))->getZExtValue();
            varName = getInstructionString(*dyn_cast<Instruction>(I.getOperand(0)));
            varRange2 = std::make_pair(value, value);
            instrRangeMap[instrMap[varName]] = addOp(originalRange, varRange2);
        }
        
    }
}

void backwardMul(Instruction &I, std::map<Instruction *, std::pair<int, int>> &instrRangeMap, std::map<std::string, Instruction *> &instrMap){
    std::pair<int, int> originalRange = instrRangeMap[instrMap[getInstructionString(I)]];
    std::pair<int, int> varRange1, varRange2;

    if(!isa<llvm::ConstantInt>(I.getOperand(0)) && !isa<llvm::ConstantInt>(I.getOperand(1))) {
        std::string varName1 = getInstructionString(*dyn_cast<Instruction>(I.getOperand(0)));
        varRange1 = instrRangeMap[instrMap[varName1]];
        std::string varName2 = getInstructionString(*dyn_cast<Instruction>(I.getOperand(1)));
        varRange2 = instrRangeMap[instrMap[varName2]];
        std::pair<int, int> divRange1 = divOp(originalRange, varRange2);
        std::pair<int, int> divRange2 = divOp(originalRange, varRange1);
        instrRangeMap[instrMap[varName1]] = std::make_pair(std::max(varRange1.first, divRange1.first), std::min(varRange2.second, divRange1.second));
        instrRangeMap[instrMap[varName2]] = std::make_pair(std::max(varRange2.first, divRange2.first), std::min(varRange2.second, divRange2.second));
    } else {
        std::string varName;
        if(isa<llvm::ConstantInt>(I.getOperand(0))){
            int value = dyn_cast<llvm::ConstantInt>(I.getOperand(0))->getZExtValue();
            varName = getInstructionString(*dyn_cast<Instruction>(I.getOperand(1)));
            varRange2 = std::make_pair(value, value);
        } else if (isa<llvm::ConstantInt>(I.getOperand(1))){
            int value = dyn_cast<llvm::ConstantInt>(I.getOperand(1))->getZExtValue();
            varName = getInstructionString(*dyn_cast<Instruction>(I.getOperand(0)));
            varRange2 = std::make_pair(value, value);
        }
        instrRangeMap[instrMap[varName]] = divOp(originalRange, varRange2);
    }
}

void backwardLoad(Instruction &I, std::map<Instruction *, std::pair<int, int>> &instrRangeMap, std::map<std::string, Instruction *> &instrMap){
    std::string varName = getInstructionString(*dyn_cast<Instruction>(I.getOperand(0)));
    if (!checkEmpty(instrRangeMap[instrMap[varName]])) { ;
        if (checkEmpty(instrRangeMap[instrMap[getInstructionString(I)]])) {
            instrRangeMap[instrMap[varName]].first = limitRange(POS_LIMIT);
            instrRangeMap[instrMap[varName]].second = limitRange(NEG_LIMIT);
        } else {
            instrRangeMap[instrMap[varName]].first = limitRange(std::max(instrRangeMap[instrMap[varName]].first, instrRangeMap[instrMap[getInstructionString(I)]].first));
            instrRangeMap[instrMap[varName]].second = limitRange(std::min(instrRangeMap[instrMap[varName]].second, instrRangeMap[instrMap[getInstructionString(I)]].second));
        }
    }
}

void backwardStore(Instruction &I, std::map<Instruction *, std::pair<int, int>> &instrRangeMap, std::map<std::string, Instruction *> &instrMap) {
    if (!isa<llvm::ConstantInt>(I.getOperand(0))) {
        std::string varName1 = getInstructionString(*dyn_cast<Instruction>(I.getOperand(0)));
        std::string varName2 = getInstructionString(*dyn_cast<Instruction>(I.getOperand(1)));
        instrRangeMap[instrMap[varName1]] = instrRangeMap[instrMap[varName2]];
    }
}

void allocaOperation(Instruction &I, std::map<Instruction *, std::pair<int, int>> &instrRangeMap, std::map<std::string, Instruction *> &instrMap) {
    // init all the variables
    instrRangeMap[instrMap[getInstructionString(I)]] = std::make_pair(NEG_LIMIT, POS_LIMIT);
}

void storeOperation(Instruction &I, std::map<Instruction *, std::pair<int, int>> &instrRangeMap, std::map<std::string, Instruction *> &instrMap) {
    if (isa<llvm::ConstantInt>(I.getOperand(0))) {
        int value = dyn_cast<llvm::ConstantInt>(I.getOperand(0))->getZExtValue();
        std::string varName = getInstructionString(*dyn_cast<Instruction>(I.getOperand(1)));
        instrRangeMap[instrMap[varName]] = std::make_pair(value, value);
    } else {
        std::string varName1 = getInstructionString(*dyn_cast<Instruction>(I.getOperand(0)));
        std::string varName2 = getInstructionString(*dyn_cast<Instruction>(I.getOperand(1)));
        instrRangeMap[instrMap[varName2]] = instrRangeMap[instrMap[varName1]];
    }
}

void loadOperation(Instruction &I, std::map<Instruction *, std::pair<int, int>> &instrRangeMap, std::map<std::string, Instruction *> &instrMap) {
    std::string varName = getInstructionString(*dyn_cast<Instruction>(I.getOperand(0)));
    instrRangeMap[instrMap[getInstructionString(I)]] = instrRangeMap[instrMap[varName]];
}

void updateInterval(std::pair<int, int> &varRange1, std::pair<int, int> &varRange2, std::pair<int, int> &originalRange) {
    std::pair<int, int> xRange = addOp(varRange2, originalRange);
    varRange1.first = limitRange(std::max(xRange.first, varRange1.first));
    varRange1.second = limitRange(std::min(xRange.second, varRange1.second));
    if (varRange1.first > varRange1.second) {
        varRange1.first =  POS_LIMIT;
        varRange1.second = NEG_LIMIT;
    }
    std::pair<int, int> yMinusRange = addOp(originalRange, varRange1);
    varRange2.first = limitRange(std::max(yMinusRange.second * -1, varRange2.first));
    varRange2.second = limitRange(std::min(yMinusRange.first * -1, varRange2.second));
    if (varRange2.first > varRange2.second) {
        varRange2.first = POS_LIMIT;
        varRange2.second = NEG_LIMIT;
    }
}

void branchUpdate(Value *operand1, Value *operand2, std::pair<int, int> &R, std::map<Instruction *, std::pair<int, int>> &result,
             std::map<std::string, Instruction *> &instructionMap) {
    if (isa<llvm::ConstantInt>(operand1)) {
        auto op1_val = dyn_cast<llvm::ConstantInt>(operand1)->getZExtValue();
        std::pair<int, int> X = std::make_pair(op1_val, op1_val);
        updateInterval(X, result[instructionMap[getInstructionString(*dyn_cast<Instruction>(operand2))]], R);
    } else if (isa<llvm::ConstantInt>(operand2)) {
        auto op2_val = dyn_cast<llvm::ConstantInt>(operand2)->getZExtValue();
        std::pair<int, int> Y = std::make_pair(op2_val, op2_val);
        updateInterval(result[instructionMap[getInstructionString(*dyn_cast<Instruction>(operand1))]], Y, R);
    } else {
        updateInterval(result[instructionMap[getInstructionString(*dyn_cast<Instruction>(operand1))]],
             result[instructionMap[getInstructionString(*dyn_cast<Instruction>(operand2))]], R);
    }
}

void branchOperation(BasicBlock *BB, Instruction &I, std::map<Instruction *, std::pair<int, int>> &instrRangeMap,std::map<std::string, Instruction *> &instrMap, std::map<BasicBlock *, std::map<Instruction *, std::pair<int, int>>> &updatedBlockMap) {
    auto *currentInstr = dyn_cast<BranchInst>(&I);
    auto *successor1 = currentInstr->getSuccessor(0);
    if (currentInstr->isConditional()) {
        auto *successor2 = dyn_cast<BasicBlock>(currentInstr->getSuccessor(1));
        std::map<Instruction *, std::pair<int, int>> branch1 = instrRangeMap;
        std::map<Instruction *, std::pair<int, int>> branch2 = instrRangeMap;

        auto operand1 = dyn_cast<ICmpInst>(I.getOperand(0))->getOperand(0);
        auto operand2 = dyn_cast<ICmpInst>(I.getOperand(0))->getOperand(1);

        switch (dyn_cast<ICmpInst>(I.getOperand(0))->getSignedPredicate()) {
            case CmpInst::Predicate::ICMP_SGT : {
                std::pair<int, int> varRange_branch1 = std::make_pair(1, POS_LIMIT);
                std::pair<int, int> varRange_branch2 = std::make_pair(NEG_LIMIT, 0);
                branchUpdate(operand1, operand2, varRange_branch1, branch1, instrMap);
                branchUpdate(operand1, operand2, varRange_branch2, branch2, instrMap);
                break;
            }
            case CmpInst::Predicate::ICMP_SGE : {
                std::pair<int, int> varRange_branch1 = std::make_pair(0, POS_LIMIT);
                std::pair<int, int> varRange_branch2 = std::make_pair(NEG_LIMIT, -1);
                branchUpdate(operand1, operand2, varRange_branch1, branch1, instrMap);
                branchUpdate(operand1, operand2, varRange_branch2, branch2, instrMap);
                break;
            }
            case CmpInst::Predicate::ICMP_SLT : {
                std::pair<int, int> varRange_branch1 = std::make_pair(NEG_LIMIT, -1);
                std::pair<int, int> varRange_branch2 = std::make_pair(0, POS_LIMIT);
                branchUpdate(operand1, operand2, varRange_branch1, branch1, instrMap);
                branchUpdate(operand1, operand2, varRange_branch2, branch2, instrMap);
                break;
            }
            case CmpInst::Predicate::ICMP_SLE : {
                std::pair<int, int> varRange_branch1 = std::make_pair(NEG_LIMIT, 0);
                std::pair<int, int> varRange_branch2 = std::make_pair(1, POS_LIMIT);
                branchUpdate(operand1, operand2, varRange_branch1, branch1, instrMap);
                branchUpdate(operand1, operand2, varRange_branch2, branch2, instrMap);
                break;
            }
            case CmpInst::Predicate::ICMP_EQ : {
                std::pair<int, int> varRange_branch1 = std::make_pair(0, 0);
                std::pair<int, int> varRange_branch2 = std::make_pair(NEG_LIMIT, POS_LIMIT);
                branchUpdate(operand1, operand2, varRange_branch1, branch1, instrMap);
                branchUpdate(operand1, operand2, varRange_branch2, branch2, instrMap);
                break;
            }
            case CmpInst::Predicate::ICMP_NE : {
                std::pair<int, int> varRange_branch1 = std::make_pair(NEG_LIMIT, POS_LIMIT);
                std::pair<int, int> varRange_branch2 = std::make_pair(0, 0);
                branchUpdate(operand1, operand2, varRange_branch1, branch1, instrMap);
                branchUpdate(operand1, operand2, varRange_branch2, branch2, instrMap);
                break;
            }
            default: {
                break;
            }
        }
        // if(varRange_branch1 != std::make_pair(NEG_LIMIT, POS_LIMIT) && varRange_branch2 != std::make_pair(NEG_LIMIT, POS_LIMIT)){
        //     if(isa<llvm::ConstantInt>(operand1)){
        //         auto constant1 = dyn_cast<llvm::ConstantInt>(operand1)->getZExtValue();
        //         std::pair<int, int> range1 = std::make_pair(constant1, constant1);
        //         updateInterval(range1, branch1[instrMap[getInstructionString(*dyn_cast<Instruction>(operand2))]], varRange_branch1);
        //         updateInterval(range1, branch2[instrMap[getInstructionString(*dyn_cast<Instruction>(operand2))]], varRange_branch2);
        //     } else if (isa<llvm::ConstantInt>(operand2)) {
        //         auto constant2 = dyn_cast<llvm::ConstantInt>(operand2)->getZExtValue();
        //         std::pair<int, int> range1 = std::make_pair(constant2, constant2);
        //         updateInterval(branch1[instrMap[getInstructionString(*dyn_cast<Instruction>(operand1))]], range1, varRange_branch1);
        //         updateInterval(branch2[instrMap[getInstructionString(*dyn_cast<Instruction>(operand1))]], range1, varRange_branch2);
        //     } else {
        //         updateInterval(branch1[instrMap[getInstructionString(*dyn_cast<Instruction>(operand1))]], branch1[instrMap[getInstructionString(*dyn_cast<Instruction>(operand2))]], varRange_branch1);
        //         updateInterval(branch2[instrMap[getInstructionString(*dyn_cast<Instruction>(operand1))]], branch1[instrMap[getInstructionString(*dyn_cast<Instruction>(operand2))]], varRange_branch2);
        //     }
        // }
        auto iterator = BB->rend();    
        while (true) {
            iterator--;
            if (iterator == BB->rbegin()) {
                break;
            }
            switch (iterator->getOpcode()) {
                case Instruction::Add: {
                    backwardAdd(*iterator, branch1, instrMap);
                    backwardAdd(*iterator, branch2, instrMap);
                }
                case Instruction::Sub: {
                    backwardSub(*iterator, branch1, instrMap);
                    backwardSub(*iterator, branch2, instrMap);
                    break;
                }
                case Instruction::Mul: {
                    backwardMul(*iterator, branch1, instrMap);
                    backwardMul(*iterator, branch2, instrMap);
                    break;
                }
                case Instruction::Load: {
                    backwardLoad(*iterator, branch1, instrMap);
                    backwardLoad(*iterator, branch2, instrMap);
                    break;
                }
                case Instruction::Store: {
                    backwardStore(*iterator, branch1, instrMap);
                    backwardStore(*iterator, branch2, instrMap);
                    break;
                }
                case Instruction::SRem: 
                case Instruction::Alloca: 
                default: 
                    break;
            }
        }
        updatedBlockMap[successor1] = branch1;
        updatedBlockMap[successor2] = branch2;
    } else {
        updatedBlockMap[successor1] = instrRangeMap;
        return;
    }
}




// void addOperation(Instruction &I, std::map<Instruction *, interval> &instrRangeMap, std::map<std::string, Instruction *> &instrMap) {
//     std::string instructionName = getInstructionString(I);
//     Value *op1 = I.getOperand(0);
//     Value *op2 = I.getOperand(1);

//     if (isa<llvm::ConstantInt>(op2)) {
//         auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
//         std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
//         interval interval_op1 = instrRangeMap[instrMap[op1_str]];
//         instrRangeMap[instrMap[instructionName]] = addOp(interval_op1, interval(op2_val, op2_val));
//     } else if (isa<llvm::ConstantInt>(op1)) {
//         auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
//         std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
//         interval interval_op2 = instrRangeMap[instrMap[op2_str]];
//         instrRangeMap[instrMap[instructionName]] = addOp(interval_op2, interval(op1_val, op1_val));
//     } else {
//         interval interval_op1 = instrRangeMap[instrMap[getInstructionString(*dyn_cast<Instruction>(op1))]];
//         interval interval_op2 = instrRangeMap[instrMap[getInstructionString(*dyn_cast<Instruction>(op2))]];
//         instrRangeMap[instrMap[instructionName]] = addOp(interval_op1, interval_op2);
//     }
// }

// void subOperation(Instruction &I, std::map<Instruction *, interval> &instrRangeMap, std::map<std::string, Instruction *> &instrMap) {
//     std::string instructionName = getInstructionString(I);
//     Value *op1 = I.getOperand(0);
//     Value *op2 = I.getOperand(1);

//     if (isa<llvm::ConstantInt>(op2)) {
//         auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
//         interval interval_op1 = instrRangeMap[instrMap[getInstructionString(*dyn_cast<Instruction>(op1))]];
//         instrRangeMap[instrMap[instructionName]] = subOp(interval_op1, interval(op2_val, op2_val));
//     } else if (isa<llvm::ConstantInt>(op1)) {
//         auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
//         interval interval_op2 = instrRangeMap[instrMap[getInstructionString(*dyn_cast<Instruction>(op2))]];
//         instrRangeMap[instrMap[instructionName]] = subOp(interval(op1_val, op1_val), interval_op2);
//     } else {
//         interval interval_op1 = instrRangeMap[instrMap[getInstructionString(*dyn_cast<Instruction>(op1))]];
//         interval interval_op2 = instrRangeMap[instrMap[getInstructionString(*dyn_cast<Instruction>(op2))]];
//         instrRangeMap[instrMap[instructionName]] = subOp(interval_op1, interval_op2);
//     }
// }

// void mulOperation(Instruction &I, std::map<Instruction *, interval> &instrRangeMap, std::map<std::string, Instruction *> &instrMap) {
//     std::string instructionName = getInstructionString(I);
//     Value *op1 = I.getOperand(0);
//     Value *op2 = I.getOperand(1);

//     if (isa<llvm::ConstantInt>(op2)) {
//         auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
//         std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
//         interval interval_op1 = instrRangeMap[instrMap[op1_str]];
//         instrRangeMap[instrMap[instructionName]] = mulOp(interval_op1, interval(op2_val, op2_val));
//     } else if (isa<llvm::ConstantInt>(op1)) {
//         auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
//         std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
//         interval interval_op2 = instrRangeMap[instrMap[op2_str]];
//         instrRangeMap[instrMap[instructionName]] = mulOp(interval_op2, interval(op1_val, op1_val));
//     } else {
//         interval interval_op1 = instrRangeMap[instrMap[getInstructionString(*dyn_cast<Instruction>(op1))]];
//         interval interval_op2 = instrRangeMap[instrMap[getInstructionString(*dyn_cast<Instruction>(op2))]];
//         instrRangeMap[instrMap[instructionName]] = mulOp(interval_op1, interval_op2);
//     }
// }

// void sremOperation(Instruction &I, std::map<Instruction *, interval> &instrRangeMap, std::map<std::string, Instruction *> &instrMap) {
//     std::string instructionName = getInstructionString(I);
//     Value *op1 = I.getOperand(0);
//     Value *op2 = I.getOperand(1);

//     if (isa<llvm::ConstantInt>(op2)) {
//         auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
//         interval interval_op1 = instrRangeMap[instrMap[getInstructionString(*dyn_cast<Instruction>(op1))]];
//         instrRangeMap[instrMap[instructionName]] = sremOp(interval_op1, interval(op2_val, op2_val));
//     } else if (isa<llvm::ConstantInt>(op1)) {
//         auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
//         interval interval_op2 = instrRangeMap[instrMap[getInstructionString(*dyn_cast<Instruction>(op2))]];
//         instrRangeMap[instrMap[instructionName]] = sremOp(interval(op1_val, op1_val), interval_op2);
//     } else {
//         interval interval_op1 = instrRangeMap[instrMap[getInstructionString(*dyn_cast<Instruction>(op1))]];
//         interval interval_op2 = instrRangeMap[instrMap[getInstructionString(*dyn_cast<Instruction>(op2))]];
//         instrRangeMap[instrMap[instructionName]] = sremOp(interval_op1, interval_op2);
//     }
// }
