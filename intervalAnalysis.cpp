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
    if (checkEmpty(a)) {
        return a;
    }
    if ( checkEmpty(b)) {
        return b;
    }
    int min = NEG_LIMIT;
    int max = POS_LIMIT;
    if (a.first != NEG_LIMIT && b.first != NEG_LIMIT) {
        min = a.first + b.first;;
    } 
    if (a.second != POS_LIMIT && b.second != POS_LIMIT) {
        max = a.second + b.second;
    }
    return std::make_pair(min, max);
}

std::pair<int, int> subOp(std::pair<int, int> a, std::pair<int, int> b) {
    if (checkEmpty(a)) {
        return a;
    }
    if ( checkEmpty(b)) {
        return b;
    }
    int min = NEG_LIMIT;
    int max = POS_LIMIT;
    if (a.first != NEG_LIMIT && b.second != POS_LIMIT) {
        min = a.first - b.second;
    }
    if (a.second != POS_LIMIT && b.first != NEG_LIMIT) {
        max = a.second - b.first;
    }
    return std::make_pair(min, max);
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
        if (interval1.first == POS_LIMIT || interval2.first == POS_LIMIT || interval1.first > interval2.second || interval2.first > interval1.second) {
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
bool intervalBlockAnalysis(BasicBlock *BB,
                             std::map<std::map<Instruction *, varInterval> *, std::map<Instruction *, varInterval>> &input,
                             std::map<std::string, Instruction *> &instructionMap,
                             std::map<BasicBlock *, std::map<std::map<Instruction *, varInterval> *, std::map<Instruction *, varInterval>>> &contextAnalysisMap,
                             std::map<BasicBlock *, std::map<std::map<Instruction *, varInterval> *, std::map<Instruction *, varInterval>>> &result);


void printBasicBlockContext(std::map<BasicBlock *, std::vector<std::map<Instruction *, varInterval>>> context) {
    for (auto &p : context) {
        std::cout << getBasicBlockLabel(p.first) << std::endl;
        for (auto &pp : p.second) {
            std::cout << "Case:" << std::endl;
            for (auto &ppp : pp) {
                std::cout << getInstructionString(*(ppp.first)) << "  >>  " << ppp.second.getIntervalString()
                          << std::endl;
            }
        }
    }
}

void printcontextCombination(std::vector<std::map<Instruction *, varInterval>> &contextCombination) {
    for (auto &combination : contextCombination) {
        std::cout << "================Context=============" << std::endl;
        for (auto &combination_pair : combination) {
            std::cout << getInstructionString(*(combination_pair.first)) << "  >>  "
                      << combination_pair.second.getIntervalString()
                      << std::endl;
        }
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
    std::map<BasicBlock *, std::vector<std::map<Instruction *, std::pair<int, int>>>> varIntervalMap;
    std::map<BasicBlock *, std::vector<std::map<Instruction *, std::pair<int, int>>>> globalVarIntervalMap;

    std::map<BasicBlock *, std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>>> intervalAnalysisMap;
    std::stack<std::pair<BasicBlock *, std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>>>> intervalTraversalStack;

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

    for(auto &row : analysisMap) {
        BasicBlock *BB = row.first;
        std::map<Instruction *, std::pair<int, int>> instrRangeMap = row.second;
        std::map<BasicBlock *, std::map<Instruction *, std::pair<int, int>>> updatedBlockMap;

        for(auto &instr : *BB) {
            if(intr.getOpcode() == Instruction::Br) {
                branchOperation(BB, instr, instrRangeMap, varMap, updatedBlockMap);
                if(updatedBlockMap.size() > 1) {
                    for(auto &instrMap : updatedBlockMap){
                        varIntervalMap[BB].push_back(instrMap.second)
                    }
                    for (auto iter = varIntervalMap[BB][0].begin(); iter != varIntervalMap[BB][0].end();) {
                        bool match = false;
                        for (auto iter1 = varIntervalMap[BB][1].begin(); iter1 != varIntervalMap[BB][1].end();) {
                            if (iter->second.getIntervalString() == iter1->second.getIntervalString()) {
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

                    // for(int i =0; i < varIntervalMap[BB][0].size(); i++){
                    //     for(int j = 0; j < varIntervalMap[BB][1].size(); j++){
                    //         if(varIntervalMap[BB][0][i].second.getIntervalString() == varIntervalMap[BB][1][j].second.getIntervalString()) {

                    //         }
                    //     }
                    // }

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
    }

    printBasicBlockContext(varIntervalMap);

    for(auto varInterval : varIntervalMap) {
        // std::vector<std::map<Instruction*, varInterval>> size not equal to 0 and map size not equal to 0
        if (varInterval.second.size() != 0 && varInterval.second[0].size() != 0) {
            if(globalVarIntervalMap.size() == 0) {
                globalVarIntervalMap = varInterval.second;
            } else {
                std::map<BasicBlock *, std::vector<std::map<Instruction *, std::pair<int, int>>>> copy = globalVarIntervalMap;
                globalVarIntervalMap.clear();
                for(auto instrMap : varInterval.second) {
                    for(auto globalInstrMap : copy) {
                        for(auto iter = instrMap.begin(); iter != instrMap.end(); iter++) {
                            if (globalInstrMap.find(iter->first) != globalInstrMap.end()) {
                                std::pair<int, int> intersection = intersection(globalInstrMap.find(iter->first)->second,  iter->second);
                                globalInstrMap.insert(std::make_pair(iter->first,intersection));
                            } else {
                                globalInstrMap.insert(*iter);
                            }
                        }
                    }
                }
            }
        }
    }

    printcontextCombination(globalVarIntervalMap);

    std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>> emptyIntervalSet;
    for (auto &varInterval : globalVarIntervalMap) {
        std::map<Instruction *, std::pair<int, int>> emptySet;
        emptyIntervalSet.insert(std::make_pair(&varInterval, varInterval));
    }
    intervalTraversalStack.push(std::make_pair(entryBB, emptyIntervalSet));

    int count = 0;
    while (!intervalTraversalStack.empty() && count < 10000) {
        std::map<BasicBlock *, std::map<std::map<Instruction *, std::pair<int, int>> *, std::map<Instruction *, std::pair<int, int>>>> updatedBlockIntervalMap;
        auto bMap = intervalTraversalStack.top();
        intervalTraversalStack.pop();

        bool changed = intervalBlockAnalysis(bMap.first, bMap.second, varMap, intervalAnalysisMap, updatedBlockIntervalMap);

        if (changed) {
            for (auto &p : updatedBlockIntervalMap) {
                intervalTraversalStack.push(p);
            }
        }
        count++;
    }

    printAnalysisMapWithContext(intervalAnalysisMap);

    // We now print the analysis results:
    for (auto& row : analysisMap) {
        row.first->printAsOperand(errs(), false);
        errs() << ": \n";
        for (auto instrMap = row.second.begin(); instrMap != row.second.end(); instrMap++) {
            std::string varName = (*instrMap).first->getName().str().c_str();
            if (varName.size() == 0) {
                continue;
            }
            for (auto instrMapItr = instrMap; instrMapItr != row.second.end(); instrMapItr++) {
                std::string varName2 = (*instrMapItr).first->getName().str().c_str();
                if (varName2.size() == 0 || varName == varName2) {
                    continue;
                }
                int v1_range[2] = {(*instrMap).second.first, (*instrMap).second.second};
                int v2_range[2] = {(*instrMapItr).second.first, (*instrMapItr).second.second};
                // std::cout << "(" << v1_range[0] << ", " << v1_range[1] << ", " << v2_range[0] <<", " << v2_range[1] << ") = Infinite" << std::endl;
                if (v1_range[0] == NEG_LIMIT || v1_range[1] == POS_LIMIT || v2_range[0] == NEG_LIMIT || v2_range[1] == POS_LIMIT) {
                    std::cout << "(" << varName << ", " << varName2 << ") = Infinite" << std::endl;
                } else {
                    int min = std::abs(v1_range[0] - v2_range[1]);
                    int max = std::abs(v1_range[1] - v2_range[0]);
                    std::cout << "(" << varName << ", " << varName2 << ") = " << std::to_string(std::max(min, max)) << std::endl;
                }

            }
        }
    }
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
            analysisMap[p.first].second = limitRange(std::min(p.second.first, analysisMap[p.first].first));
            analysisMap[p.first].second = limitRange(std::max(p.second.second, analysisMap[p.first].second));
            changed = true;
        }
    }
    return changed;
}

bool intervalBlockAnalysis

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
            instrRangeMap[instrMap[varName]].first = limitRange(NEG_LIMIT);
            instrRangeMap[instrMap[varName]].second = limitRange(POS_LIMIT);
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

void branchOperation(BasicBlock *BB, Instruction &I, std::map<Instruction *, std::pair<int, int>> &instrRangeMap,std::map<std::string, Instruction *> &instrMap, std::map<BasicBlock *, std::map<Instruction *, std::pair<int, int>>> &updatedBlockMap) {
    auto *currentInstr = dyn_cast<BranchInst>(&I);
    auto *successor1 = currentInstr->getSuccessor(0);
    if (currentInstr->isConditional()) {
        auto *successor2 = dyn_cast<BasicBlock>(currentInstr->getSuccessor(1));
        std::map<Instruction *, std::pair<int, int>> branch1 = instrRangeMap;
        std::map<Instruction *, std::pair<int, int>> branch2 = instrRangeMap;

        auto operand1 = dyn_cast<ICmpInst>(I.getOperand(0))->getOperand(0);
        auto operand2 = dyn_cast<ICmpInst>(I.getOperand(0))->getOperand(1);

        std::pair<int, int> varRange_branch1 = std::make_pair(NEG_LIMIT, POS_LIMIT);
        std::pair<int, int> varRange_branch2 = std::make_pair(NEG_LIMIT, POS_LIMIT);

        switch (dyn_cast<ICmpInst>(I.getOperand(0))->getSignedPredicate()) {
            case CmpInst::Predicate::ICMP_SGT : {
                varRange_branch1 = std::make_pair(1, POS_LIMIT);
                varRange_branch2 = std::make_pair(NEG_LIMIT, 0);
                break;
            }
            case CmpInst::Predicate::ICMP_SGE : {
                varRange_branch1 = std::make_pair(0, POS_LIMIT);
                varRange_branch2 = std::make_pair(NEG_LIMIT, -1);
                break;
            }
            case CmpInst::Predicate::ICMP_SLT : {
                varRange_branch1 = std::make_pair(NEG_LIMIT, -1);
                varRange_branch2 = std::make_pair(0, POS_LIMIT);
                break;
            }
            case CmpInst::Predicate::ICMP_SLE : {
                varRange_branch1 = std::make_pair(NEG_LIMIT, 0);
                varRange_branch2 = std::make_pair(1, POS_LIMIT);
                break;
            }
            case CmpInst::Predicate::ICMP_EQ : {
                varRange_branch1 = std::make_pair(0, 0);
                varRange_branch2 = std::make_pair(NEG_LIMIT, POS_LIMIT);
                break;
            }
            case CmpInst::Predicate::ICMP_NE : {
                varRange_branch1 = std::make_pair(NEG_LIMIT, POS_LIMIT);
                varRange_branch2 = std::make_pair(0, 0);
                break;
            }
            default: {
                break;
            }
        }
        if(varRange_branch1 != std::make_pair(NEG_LIMIT, POS_LIMIT) && varRange_branch2 != std::make_pair(NEG_LIMIT, POS_LIMIT)){
            if(isa<llvm::ConstantInt>(operand1)){
                auto constant1 = dyn_cast<llvm::ConstantInt>(operand1)->getZExtValue();
                std::pair<int, int> range1 = std::make_pair(constant1, constant1);
                updateInterval(range1, branch1[instrMap[getInstructionString(*dyn_cast<Instruction>(operand2))]], varRange_branch1);
                updateInterval(range1, branch2[instrMap[getInstructionString(*dyn_cast<Instruction>(operand2))]], varRange_branch2);
            } else if (isa<llvm::ConstantInt>(operand2)) {
                auto constant2 = dyn_cast<llvm::ConstantInt>(operand2)->getZExtValue();
                std::pair<int, int> range1 = std::make_pair(constant2, constant2);
                updateInterval(branch1[instrMap[getInstructionString(*dyn_cast<Instruction>(operand1))]], range1, varRange_branch1);
                updateInterval(branch2[instrMap[getInstructionString(*dyn_cast<Instruction>(operand1))]], range1, varRange_branch2);
            } else {
                updateInterval(branch1[instrMap[getInstructionString(*dyn_cast<Instruction>(operand1))]], branch1[instrMap[getInstructionString(*dyn_cast<Instruction>(operand2))]], varRange_branch1);
                updateInterval(branch2[instrMap[getInstructionString(*dyn_cast<Instruction>(operand1))]], branch1[instrMap[getInstructionString(*dyn_cast<Instruction>(operand2))]], varRange_branch2);
            }
        }
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
    }
}