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

class interval {
    private:
        int min;
        int max;
    public:
        interval() {
            ;
        }

        interval(int lower, int upper) {
            if (lower > upper) {
                this->min = POS_LIMIT;
                this->max = NEG_LIMIT;
            }
            if (lower <= NEG_LIMIT)
                this->min = NEG_LIMIT;
            else if (lower >= POS_LIMIT)
                this->min = POS_LIMIT;
            else
                this->min = lower;

            if (upper >= POS_LIMIT)
                this->max = POS_LIMIT;
            else if (upper <= NEG_LIMIT)
                this->max = NEG_LIMIT;
            else
                this->max = upper;
        }

        int getMaxVal() {
            return this->max;
        }

        int getMinVal() {
            return this->min;
        }

        void setMinVal(int min) {
            if (min <= NEG_LIMIT)
                this->min = NEG_LIMIT;
            else if (min >= POS_LIMIT)
                this->min = POS_LIMIT;
            else
                this->min = min;

        }

        void setMaxVal(int max) {
            if (max >= POS_LIMIT)
                this->max = POS_LIMIT;
            else if (max <= NEG_LIMIT)
                this->max = NEG_LIMIT;
            else
                this->max = max;
        }

        bool isEmptyInterval() {
            return this->getMinVal() == POS_LIMIT && this->getMaxVal() == NEG_LIMIT;
        }

        std::string toString() {
            if (this->isEmptyInterval())
                return "EMPTY SET";
            return (this->getMinVal() == NEG_LIMIT ? "NEG_LIMIT" : std::to_string(this->getMinVal())) + "-" +
                   (this->getMaxVal() == POS_LIMIT ? "POS_LIMIT" : std::to_string(
                           this->getMaxVal()));
        }
};

std::map<BasicBlock *, std::vector<std::map<Instruction *, interval>>> context;

interval addOp(interval var1, interval var2) {
    if (var1.isEmptyInterval()) {
        return var1;
    }
    if (var2.isEmptyInterval()) {
        return var2;
    }
    int min = NEG_LIMIT;
    int max = POS_LIMIT;
    if (var1.getMinVal() != NEG_LIMIT && var2.getMinVal() != NEG_LIMIT) {
        min = var1.getMinVal() + var2.getMinVal();;
    } 
    if (var1.getMaxVal() != POS_LIMIT && var2.getMaxVal() != POS_LIMIT) {
        max = var1.getMaxVal() + var2.getMaxVal();
    }
    return interval(min, max);
}


interval subOp(interval var1, interval var2) {
    if (var1.isEmptyInterval()) {
        return var1;
    }
    if ( var2.isEmptyInterval()) {
        return var2;
    }
    int min = NEG_LIMIT;
    int max = POS_LIMIT;
    if (var1.getMinVal() != NEG_LIMIT && var2.getMaxVal() != POS_LIMIT) {
        min = var1.getMinVal() - var2.getMaxVal();
    }
    if (var1.getMaxVal() != POS_LIMIT && var2.getMinVal() != NEG_LIMIT) {
        max = var1.getMaxVal() - var2.getMinVal();
    }
    return interval(min, max);
}


interval mulOp(interval var1, interval var2) {
    if (var1.isEmptyInterval()) {
        return var1;
    }
    if (var2.isEmptyInterval()) {
        return var2;
    }
    std::vector<int> cmpList;
    cmpList.push_back(var1.getMinVal() * var2.getMinVal());
    cmpList.push_back(var1.getMinVal() * var2.getMaxVal());
    cmpList.push_back(var1.getMaxVal() * var2.getMinVal());
    cmpList.push_back(var1.getMaxVal() * var2.getMaxVal());
    return interval(*(std::min_element(cmpList.begin(), cmpList.end())), *(std::max_element(cmpList.begin(), cmpList.end())));
}

interval divOp(interval var1, interval var2) {
    if(var1.isEmptyInterval() || var2.isEmptyInterval() || (var2.getMinVal() == 0 && var2.getMaxVal() == 0)) {
        return interval(NEG_LIMIT, POS_LIMIT);
    }
    std::vector<int> cmpList;
    if(var2.getMinVal() == 0) {
        cmpList.push_back(var1.getMinVal());
        cmpList.push_back(var1.getMaxVal());
        cmpList.push_back(var1.getMinVal()/var2.getMaxVal());
        cmpList.push_back(var1.getMaxVal()/var2.getMaxVal());
    }else if(var2.getMaxVal() == 0) {
        cmpList.push_back(var1.getMinVal() * -1);
        cmpList.push_back(var1.getMaxVal() * -1);
        cmpList.push_back(var1.getMinVal() / var2.getMinVal());
        cmpList.push_back(var1.getMaxVal() / var2.getMinVal());
    }else if(var2.getMinVal() < 0 && var2.getMaxVal() > 0) {
        cmpList.push_back(var1.getMinVal());
        cmpList.push_back(var1.getMinVal() * -1);
        cmpList.push_back(var1.getMaxVal());
        cmpList.push_back(var1.getMaxVal() * -1);
        cmpList.push_back(var1.getMinVal()/var2.getMinVal());
        cmpList.push_back(var1.getMaxVal()/var2.getMinVal());
        cmpList.push_back(var1.getMinVal()/var2.getMaxVal());
        cmpList.push_back(var1.getMaxVal()/var2.getMaxVal());
    } else {
        cmpList.push_back(var1.getMinVal()/var2.getMinVal());
        cmpList.push_back(var1.getMaxVal()/var2.getMinVal());
        cmpList.push_back(var1.getMinVal()/var2.getMaxVal());
        cmpList.push_back(var1.getMaxVal()/var2.getMaxVal());
    }
    return interval(*(std::min_element(cmpList.begin(), cmpList.end())), *(std::max_element(cmpList.begin(), cmpList.end())));
}

interval sremOp(interval var1, interval var2) {
    if (var1.isEmptyInterval()) {
        return var1;
    }
    if ( var2.isEmptyInterval()) {
        return var2;
    }
    if (var1.getMaxVal() == POS_LIMIT && var2.getMaxVal() == POS_LIMIT) {
        return interval(0, POS_LIMIT);
    } else if (var1.getMaxVal() == POS_LIMIT) {
        return interval(0, var2.getMaxVal() - 1);
    } else if (var2.getMaxVal() == POS_LIMIT) {
        return interval(var1.getMaxVal() < var2.getMaxVal() ? var1.getMaxVal() : 0, std::max(std::abs(var1.getMaxVal()), std::abs(var1.getMaxVal())));
    } else if (var1.getMaxVal() < var2.getMaxVal()) {
        return var1;
    } else {
        return interval(0, std::min(var1.getMaxVal(), var2.getMaxVal()-1));
    }
}


interval intersection(interval v1, interval v2) {
    if (v1.getMinVal() == POS_LIMIT || v2.getMinVal() == POS_LIMIT) {
        return interval(POS_LIMIT, NEG_LIMIT);
    } else if (v1.getMinVal() > v2.getMaxVal() || v2.getMinVal() > v1.getMaxVal()) {
        return interval(POS_LIMIT, NEG_LIMIT);
    } else {
        return interval(std::max(v1.getMinVal(), v2.getMinVal()), std::min(v1.getMaxVal(), v2.getMaxVal()));
    }
}

std::string getInstructionString(Instruction &I) {
    std::string instructionStr;
    llvm::raw_string_ostream rso(instructionStr);
    I.print(rso);
    return instructionStr;
}

std::string getBasicBlockLabel(BasicBlock *BB) {
    std::string basicBlockStr;
    llvm::raw_string_ostream rso(basicBlockStr);
    rso << ">>>>Basic Block: ";
    BB->printAsOperand(rso, false);
    return basicBlockStr;
}

void comp(interval &intervalX, interval &intervalY, interval &intervalR);

void compare(Value *cmpOp1, Value *cmpOp2, interval &R, std::map<Instruction *, interval> &result,
             std::map<std::string, Instruction *> &instructionMap);

void analyzeAlloca(Instruction &I, std::map<Instruction *, interval> &blockMap,
                   std::map<std::string, Instruction *> &instructionMap, bool backward);

void analyzeAdd(Instruction &I, std::map<Instruction *, interval> &blockMap,
                std::map<std::string, Instruction *> &instructionMap, bool backward);

void analyzeSub(Instruction &I, std::map<Instruction *, interval> &blockMap,
                std::map<std::string, Instruction *> &instructionMap, bool backward);

void analyzeMul(Instruction &I, std::map<Instruction *, interval> &blockMap,
                std::map<std::string, Instruction *> &instructionMap, bool backward);

void analyzeSrem(Instruction &I, std::map<Instruction *, interval> &blockMap,
                 std::map<std::string, Instruction *> &instructionMap, bool backward);

void analyzeStore(Instruction &I, std::map<Instruction *, interval> &blockMap,
                  std::map<std::string, Instruction *> &instructionMap, bool backward);

void analyzeLoad(Instruction &I, std::map<Instruction *, interval> &blockMap,
                 std::map<std::string, Instruction *> &instructionMap, bool backward);

void analyzeBr(BasicBlock *BB, Instruction &I, std::map<Instruction *, interval> &blockMap,
               std::map<std::string, Instruction *> &instructionMap,
               std::map<BasicBlock *, std::map<Instruction *, interval>> &result);

std::map<Instruction *, interval>
joinWithContext(BasicBlock *bb, std::map<std::map<Instruction *, interval> *, std::map<Instruction *, interval>> blockMap);

bool blockAnalysis(BasicBlock *BB, std::map<Instruction *, interval> &input,
                  std::map<std::string, Instruction *> &instructionMap,
                  std::map<BasicBlock *, std::map<Instruction *, interval>> &analysisMap,
                  std::map<BasicBlock *, std::map<Instruction *, interval>> &result);

bool intervalBlockAnalysis(BasicBlock *BB,
                             std::map<std::map<Instruction *, interval> *, std::map<Instruction *, interval>> &input,
                             std::map<std::string, Instruction *> &instructionMap,
                             std::map<BasicBlock *, std::map<std::map<Instruction *, interval> *, std::map<Instruction *, interval>>> &contextAnalysisMap,
                             std::map<BasicBlock *, std::map<std::map<Instruction *, interval> *, std::map<Instruction *, interval>>> &result);

void printAnalysisMapWithContext(
        std::map<BasicBlock *, std::map<std::map<Instruction *, interval> *, std::map<Instruction *, interval>>> &contextAnalysisMap) {
    for (auto &pair : contextAnalysisMap) {
        std::cout << getBasicBlockLabel(pair.first) << "===============================" << std::endl;
        std::map<Instruction *, interval> final_result;
        final_result = joinWithContext(pair.first, pair.second);

        bool isNotEmpty = true;
        for (auto &pair : final_result) {
            if (pair.second.isEmptyInterval()) {
                isNotEmpty = false;
                break;
            }
        }
        if (!isNotEmpty) {
            continue;
        }
        for (auto &pair : final_result) {
            if(pair.first->getName().size()!=0)
                std::cout << getInstructionString(*pair.first) << "  -> " << pair.second.toString()
                      << std::endl;
        }
    }
}

void generateAnalysisReport(std::stack<std::pair<BasicBlock *, std::map<Instruction *, interval>>> &traversalStack,
                            std::map<BasicBlock *, std::map<Instruction *, interval>> &analysisMap,
                            std::map<std::string, Instruction *> &instructionMap) {
    while (!traversalStack.empty()) {
        std::map<BasicBlock *, std::map<Instruction *, interval>> result;
        auto pair = traversalStack.top();
        traversalStack.pop();

        auto changed = blockAnalysis(pair.first, pair.second, instructionMap, analysisMap, result);


        if (changed) {
            for (auto &p : result) {
                traversalStack.push(p);
            }
        }
    }
}


void generateContext(std::map<BasicBlock *, std::vector<std::map<Instruction *, interval>>> &context,
                     std::map<std::string, Instruction *> &instructionMap,
                     std::map<BasicBlock *, std::map<Instruction *, interval>> &analysisMap) {
    for (auto &p : analysisMap) {

        for (auto &pair : p.second) {
            pair.second = interval(NEG_LIMIT, POS_LIMIT);
        }

        BasicBlock *BB = p.first;
        std::map<Instruction *, interval> bbOutput = p.second;
        std::map<BasicBlock *, std::map<Instruction *, interval>> result;

        for (auto &I: *BB) {
            if (I.getOpcode() == Instruction::Br) {

                analyzeBr(BB, I, bbOutput, instructionMap, result);
  
                if (result.size() <= 1)
                    break;
   
                for (auto &pp : result) {
                    context[BB].push_back(pp.second);
                }
 
                for (auto it_0 = context[BB][0].begin(); it_0 != context[BB][0].end();) {
                    bool broke = false;
                    for (auto it_1 = context[BB][1].begin(); it_1 != context[BB][1].end();) {
                        if (it_0->second.toString() == it_1->second.toString()) {
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
                for(auto it_0 = context[BB][0].begin(); it_0 != context[BB][0].end();){
                    if(it_0->first->getName().size() == 0){
                        it_0 = context[BB][0].erase(it_0);
                    }else{
                        ++it_0;
                    }
                }
                for(auto it_1 = context[BB][1].begin(); it_1 != context[BB][1].end();){
                    if(it_1->first->getName().size() == 0){
                        it_1 = context[BB][1].erase(it_1);
                    }else{
                        ++it_1;
                    }
                }
            }
        }
    }
}

void generateContextCombination(std::vector<std::map<Instruction *, interval>> &contextCombination,
                                std::map<BasicBlock *, std::vector<std::map<Instruction *, interval>>> &context) {
    for (auto context_pair : context) {
        
        if (context_pair.second.size() == 0 || context_pair.second[0].size() == 0)
            continue;

        if (contextCombination.size() == 0) {
            contextCombination = context_pair.second;
            continue;
        }
        auto contextCombinationBackUp = contextCombination;
        contextCombination.clear();

        for (auto context_pair_map : context_pair.second) {
            
            for (auto temp_map : contextCombinationBackUp) {
               
                for (auto it = context_pair_map.begin(); it != context_pair_map.end(); ++it) {
                    if (temp_map.find(it->first) != temp_map.end()) {
                        temp_map.insert(std::make_pair(it->first,
                                                       intersection(temp_map.find(it->first)->second,
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
                              std::vector<std::map<Instruction *, interval>> &contextCombination,
                              std::map<std::string, Instruction *> &instructionMap,
                              std::map<BasicBlock *, std::map<std::map<Instruction *, interval> *, std::map<Instruction *, interval>>> &contextAnalysisMap,
                              std::stack<std::pair<BasicBlock *, std::map<std::map<Instruction *, interval> *, std::map<Instruction *, interval>>>> &contextTraversalStack) {
    std::map<std::map<Instruction *, interval> *, std::map<Instruction *, interval>> initialSet;
    for (auto &combination : contextCombination) {
        std::map<Instruction *, interval> emptySet;
        initialSet.insert(std::make_pair(&combination, combination));
    }
    contextTraversalStack.push(std::make_pair(entryBB, initialSet));

    while (!contextTraversalStack.empty()) {
        std::map<BasicBlock *, std::map<std::map<Instruction *, interval> *, std::map<Instruction *, interval>>> result;
        auto pair = contextTraversalStack.top();
        contextTraversalStack.pop();

        auto changed = intervalBlockAnalysis(pair.first, pair.second, instructionMap, contextAnalysisMap, result);

        if (changed) {
            for (auto &p : result) {
                contextTraversalStack.push(p);
            }
        }
    }
}

int main(int argc, char **argv) {

    LLVMContext &Context = getGlobalContext();
    SMDiagnostic Err;
    Module *M = ParseIRFile(argv[1], Err, Context);
    if (M == nullptr) {
        fprintf(stderr, "error: failed to load LLVM IR file \"%s\"", argv[1]);
        return EXIT_FAILURE;
    }
    Function *F = M->getFunction("main");
    BasicBlock *entryBB = &F->getEntryBlock();

    std::map<BasicBlock *, std::map<Instruction *, interval>> analysisMap;
    std::map<std::string, Instruction *> instructionMap;
    std::stack<std::pair<BasicBlock *, std::map<Instruction *, interval>>> traversalStack;
    std::map<Instruction *, interval> emptySet;
    traversalStack.push(std::make_pair(entryBB, emptySet));

    generateAnalysisReport(traversalStack, analysisMap, instructionMap);
    generateContext(context, instructionMap, analysisMap);

    std::vector<std::map<Instruction *, interval>> contextCombination;
    generateContextCombination(contextCombination, context);

    std::map<BasicBlock *, std::map<std::map<Instruction *, interval> *, std::map<Instruction *, interval>>> contextAnalysisMap;
    std::stack<std::pair<BasicBlock *, std::map<std::map<Instruction *, interval> *, std::map<Instruction *, interval>>>> contextTraversalStack;

    runBlockIntervalAnalysis(entryBB, contextCombination, instructionMap, contextAnalysisMap, contextTraversalStack);
    printAnalysisMapWithContext(contextAnalysisMap);
}

std::vector<interval> getOperandIntervals(Instruction &I,
                                             Value *op1,
                                             Value *op2,
                                             std::map<Instruction *, interval> context,
                                             std::map<Instruction *, interval> &variables,
                                             std::map<std::string, Instruction *> &instructionMap) {
    bool isInContext = context.find(&I) != context.end();
    interval interval_op1;
    interval interval_op2;
    if (isa<llvm::ConstantInt>(op2)) {
        auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
        auto op1_instr = instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))];
        interval_op1 = variables[op1_instr];
        interval_op2 = interval(op2_val, op2_val);

    } else if (isa<llvm::ConstantInt>(op1)) {
        auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
        auto op2_instr = instructionMap[getInstructionString(*dyn_cast<Instruction>(op2))];
        interval_op1 = interval(op1_val, op1_val);
        interval_op2 = variables[op2_instr];
    } else {
        auto op1_instr = instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))];
        auto op2_instr = instructionMap[getInstructionString(*dyn_cast<Instruction>(op2))];
        interval_op1 = variables[op1_instr];
        interval_op2 = variables[op2_instr];
    }
    std::vector<interval> results;
    results.push_back(interval_op1);
    results.push_back(interval_op2);
    return results;
}

bool unionAndCheckChanged(std::map<Instruction *, interval> &input,
                          std::map<Instruction *, interval> &analysisMap) {
    bool changed = false;
    for (auto &p : input) {
        if (analysisMap.find(p.first) == analysisMap.end()) {
            analysisMap[p.first] = p.second;
            changed = true;
        } else if (analysisMap[p.first].isEmptyInterval()) {
            if (!p.second.isEmptyInterval()) {
                analysisMap[p.first] = p.second;
                changed = true;
            }
        } else {
            bool subset; 
            if (p.second.getMinVal() == POS_LIMIT && p.second.getMaxVal() == NEG_LIMIT) {
                subset = true;
            }
            else if (analysisMap[p.first].getMinVal() == POS_LIMIT && analysisMap[p.first].getMaxVal() == NEG_LIMIT) {
                subset = false;
            }
            else 
                subset = p.second.getMinVal() >= analysisMap[p.first].getMinVal() && p.second.getMaxVal() <= analysisMap[p.first].getMaxVal();

            if (!subset) {
                analysisMap[p.first].setMinVal(std::min(p.second.getMinVal(), analysisMap[p.first].getMinVal()));
                analysisMap[p.first].setMaxVal(std::max(p.second.getMaxVal(), analysisMap[p.first].getMaxVal()));
                changed = true;
            }

        }
    }

    return changed;
}

void cleanUpEmpty(std::map<std::map<Instruction *, interval> *, std::map<Instruction *, interval>> &blockMap) {
    for (auto &pair : blockMap) {
        bool containsEmpty = false;

        for (auto &c : *pair.first) {
            pair.second[c.first] = intersection(c.second, pair.second[c.first]);
        }

        for (auto &v : pair.second) {
            if (v.second.isEmptyInterval()) {
                containsEmpty = true;
                break;
            }
        }
        if (containsEmpty) {
            for (auto &v : pair.second) {
                v.second = interval(POS_LIMIT, NEG_LIMIT);
            }
        }
    }
}

std::map<Instruction *, interval> joinWithContext(BasicBlock *bb, std::map<std::map<Instruction *, interval> *, std::map<Instruction *, interval>> blockMap) {
    std::map<Instruction *, interval> unionSet;
    if (blockMap.size() == 0) return unionSet;
    std::vector<Instruction *> intersetInstructionSet;
    for (auto it = blockMap.begin(); it != blockMap.end(); ++it) {
        if (it == blockMap.begin()) {
            for (auto p : it->second) intersetInstructionSet.push_back(p.first);
        } else {
            std::vector<Instruction *> tempSet;
            for (auto p : it->second) 
                tempSet.push_back(p.first);

            std::vector<Instruction*> tempVector;

            sort(intersetInstructionSet.begin(), intersetInstructionSet.end());
            sort(tempSet.begin(), tempSet.end());

            set_intersection(intersetInstructionSet.begin(),intersetInstructionSet.end(),tempSet.begin(),tempSet.end(),back_inserter(tempVector));
            intersetInstructionSet = tempVector;
        }
    }
    for (auto it = blockMap.begin(); it != blockMap.end(); ++it) {
        if (it == blockMap.begin()) {
            for (auto &element : intersetInstructionSet) {
                    unionSet.insert(std::make_pair(element, it->second[element]));

            }
        } else {
            for (auto &element : intersetInstructionSet) {
                    unionSet[element] = interval(std::min(unionSet[element].getMinVal(), it->second[element].getMinVal()), std::max(unionSet[element].getMaxVal(), it->second[element].getMaxVal()));
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

void operatorWithContext(BasicBlock *bb, Instruction &I,
                         std::map<std::map<Instruction *, interval> *, std::map<Instruction *, interval>> &blockMap,
                         std::map<std::string, Instruction *> &instructionMap, std::string op) {
    if (blockMap.begin()->first->find(&I) != blockMap.begin()->first->end()) {
        auto join = joinWithContext(bb, blockMap);
        for (auto &pair :blockMap) {
            auto context = *pair.first;
            auto result = getOperandIntervals(I, I.getOperand(0), I.getOperand(1), *pair.first, join, instructionMap);
            interval temp = interval(POS_LIMIT, NEG_LIMIT);
            if(op == "add") {
                temp = addOp(result[0], result[1]);
            } else if (op == "sub") {
                temp = subOp(result[0], result[1]);
            } else if (op == "mul") {
                temp = mulOp(result[0], result[1]);
            } else if (op == "srem") {
                temp = sremOp(result[0], result[1]);
            }
            if (intersection(temp, context[&I]).isEmptyInterval())
                for (auto &var : pair.second)
                    var.second = interval(POS_LIMIT, NEG_LIMIT);
            else {
                pair.second = join;
                pair.second[&I] = intersection(temp, context[&I]);
            }
        }
    } else {
        for (auto &pair : blockMap) {
            auto context = *pair.first;
            auto result = getOperandIntervals(I, I.getOperand(0), I.getOperand(1), *pair.first, pair.second, instructionMap);
            interval temp = interval(POS_LIMIT, NEG_LIMIT);
            if(op == "add") {
                temp = addOp(result[0], result[1]);
            } else if (op == "sub") {
                temp = subOp(result[0], result[1]);
            } else if (op == "mul") {
                temp = mulOp(result[0], result[1]);
            } else if (op == "srem") {
                temp = sremOp(result[0], result[1]);
            }
            pair.second[&I] = temp;
        }
    }
    cleanUpEmpty(blockMap);
}

void analyzeStoreWithContext(BasicBlock *bb, Instruction &I,
                             std::map<std::map<Instruction *, interval> *, std::map<Instruction *, interval>> &blockMap,
                             std::map<std::string, Instruction *> &instructionMap) {
    Value *op1 = I.getOperand(0);
    auto op2_instr = instructionMap[getInstructionString(*dyn_cast<Instruction>(I.getOperand(1)))];
    interval temp;
    if (blockMap.begin()->first->find(op2_instr) != blockMap.begin()->first->end()) {

        auto join = joinWithContext(bb, blockMap);


        if (isa<llvm::ConstantInt>(op1)) {
            auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
            temp = interval(op1_val, op1_val);
        } else {
            auto op1_instr = instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))];
            temp = join[op1_instr];
        }
        for (auto &pair :blockMap) {
            auto context = *pair.first;
            if (intersection(temp, context[op2_instr]).isEmptyInterval()) {
                for (auto &var : pair.second)
                    var.second = interval(POS_LIMIT, NEG_LIMIT);
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
                temp = interval(op1_val, op1_val);
            } else {
                auto op1_instr = instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))];
                temp = pair.second[op1_instr];
            }
            pair.second[op2_instr] = temp;
        }
    }

    cleanUpEmpty(blockMap);
}

void analyzeLoadWithContext(BasicBlock *bb, Instruction &I,
                            std::map<std::map<Instruction *, interval> *, std::map<Instruction *, interval>> &blockMap,
                            std::map<std::string, Instruction *> &instructionMap) {
    auto *op_instruction = dyn_cast<Instruction>(I.getOperand(0));
    if (blockMap.begin()->first->find(&I) != blockMap.begin()->first->end()) {
        auto join = joinWithContext(bb, blockMap);
        for (auto &pair :blockMap) {
            auto context = *pair.first;
            interval temp = pair.second[instructionMap[getInstructionString(*op_instruction)]];
            if (intersection(temp, context[&I]).isEmptyInterval()) {
                for (auto &var : pair.second)
                    var.second = interval(POS_LIMIT, NEG_LIMIT);
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
    cleanUpEmpty(blockMap);
}

void analyzeAllocaWithContext(BasicBlock *bb, Instruction &I,
                              std::map<std::map<Instruction *, interval> *, std::map<Instruction *, interval>> &blockMap,
                              std::map<std::string, Instruction *> &instructionMap) {
    if (blockMap.begin()->first->find(&I) != blockMap.begin()->first->end()) {
        for (auto &pair :blockMap)
            pair.second = joinWithContext(bb, blockMap);
    } else {
        for (auto &pair : blockMap)
            pair.second[&I] = interval(NEG_LIMIT, POS_LIMIT);
    }
    cleanUpEmpty(blockMap);

}

void analyzeBrWithContext(BasicBlock *BB, Instruction &I,
                          std::map<std::map<Instruction *, interval> *, std::map<Instruction *, interval>> &blockMap,
                          std::map<std::string, Instruction *> &instructionMap,
                          std::map<BasicBlock *, std::map<std::map<Instruction *, interval> *, std::map<Instruction *, interval>>> &result) {
    auto *branchInst = dyn_cast<BranchInst>(&I);
    auto *bb_op1 = branchInst->getSuccessor(0);
    if (!(branchInst->isConditional())) {
        result[bb_op1] = blockMap;
        return;
    }
    auto *bb_op2 = dyn_cast<BasicBlock>(branchInst->getSuccessor(1));

    std::map<std::map<Instruction *, interval> *, std::map<Instruction *, interval>> total_branch1 = blockMap;
    std::map<std::map<Instruction *, interval> *, std::map<Instruction *, interval>> total_branch2 = blockMap;


    auto branch_1_context = context[BB][0];
    auto branch_2_context = context[BB][1];


    for (auto &pair : total_branch1) {
        bool emptyBranch1 = false;
        for (auto &context_pair : *pair.first) {
            if (branch_1_context.find(context_pair.first) != branch_1_context.end() &&
                intersection(branch_1_context[context_pair.first], context_pair.second).isEmptyInterval()) {
                emptyBranch1 = true;
                break;
            }
        }
        if(emptyBranch1){
            for(auto &variable_pair : pair.second){
                variable_pair.second = interval(POS_LIMIT, NEG_LIMIT);
            }
        }
    }

    for (auto &pair : total_branch2) {
        bool emptyBranch2 = false;
        for (auto &context_pair : *pair.first) {
            if (branch_2_context.find(context_pair.first) != branch_2_context.end() &&
                intersection(branch_2_context[context_pair.first], context_pair.second).isEmptyInterval()) {
                emptyBranch2 = true;
                break;
            }
        }
        if(emptyBranch2){
            for(auto &variable_pair : pair.second){
                variable_pair.second = interval(POS_LIMIT, NEG_LIMIT);
            }
        }
    }

    result[bb_op1] = total_branch1;
    result[bb_op2] = total_branch2;

}


bool unionAndCheckChangedWithContext(
        std::map<std::map<Instruction *, interval> *, std::map<Instruction *, interval>> &input,
        std::map<std::map<Instruction *, interval> *, std::map<Instruction *, interval>> &analysisMap) {
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
            } else if (analysisMap_result[p.first].isEmptyInterval()) {
                if (!p.second.isEmptyInterval()) {
                    analysisMap[context.first][p.first] = p.second;
                    changed = true;
                }
            } else {
                bool subset; 
                if (p.second.getMinVal() == POS_LIMIT && p.second.getMaxVal() == NEG_LIMIT) {
                    subset = true;
                }
                else if (analysisMap_result[p.first].getMinVal() == POS_LIMIT && analysisMap_result[p.first].getMaxVal() == NEG_LIMIT) {
                    subset = false;
                }
                else 
                    subset = p.second.getMinVal() >= analysisMap_result[p.first].getMinVal() && p.second.getMaxVal() <= analysisMap_result[p.first].getMaxVal();

                if (!subset) {
                    analysisMap[context.first][p.first].setMinVal(
                            std::min(p.second.getMinVal(), analysisMap_result[p.first].getMinVal()));
                    analysisMap[context.first][p.first].setMaxVal(
                            std::max(p.second.getMaxVal(), analysisMap_result[p.first].getMaxVal()));
                    changed = true;
                }

            } 
        }
    }
    return changed;
}

bool intervalBlockAnalysis(BasicBlock *BB,
                             std::map<std::map<Instruction *, interval> *, std::map<Instruction *, interval>> &input,
                             std::map<std::string, Instruction *> &instructionMap,
                             std::map<BasicBlock *, std::map<std::map<Instruction *, interval> *, std::map<Instruction *, interval>>> &contextAnalysisMap,
                             std::map<BasicBlock *, std::map<std::map<Instruction *, interval> *, std::map<Instruction *, interval>>> &result) {
    for (auto &I: *BB) {
        switch (I.getOpcode()) {
            case Instruction::Add: {
                operatorWithContext(BB, I, input, instructionMap, "add");
                break;
            }
            case Instruction::Sub: {
                operatorWithContext(BB, I, input, instructionMap, "sub");
                break;
            }
            case Instruction::Mul: {
                operatorWithContext(BB, I, input, instructionMap, "mul");
                break;
            }
            case Instruction::SRem: {
                operatorWithContext(BB, I, input, instructionMap, "srem");
                break;
            }
            case Instruction::Br: {
                analyzeBrWithContext(BB, I, input, instructionMap, result);
                return unionAndCheckChangedWithContext(input, contextAnalysisMap[BB]);
            }
            case Instruction::Alloca: {
                analyzeAllocaWithContext(BB, I, input, instructionMap);
                break;
            }
            case Instruction::Load: {
                analyzeLoadWithContext(BB, I, input, instructionMap);
                break;
            }
            case Instruction::Store: {
                analyzeStoreWithContext(BB, I, input, instructionMap);
                break;
            }
            case Instruction::Ret: {
                return unionAndCheckChangedWithContext(input, contextAnalysisMap[BB]);
            }
            default: {
                break;
            }
        }
    }
    return false;


}

bool blockAnalysis(BasicBlock *BB, std::map<Instruction *, interval> &input,
                  std::map<std::string, Instruction *> &instructionMap,
                  std::map<BasicBlock *, std::map<Instruction *, interval>> &analysisMap,
                  std::map<BasicBlock *, std::map<Instruction *, interval>> &result) {
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
                break;
            }
        }
    }
    return false;
}

void analyzeAdd(Instruction &I, std::map<Instruction *, interval> &blockMap,
                std::map<std::string, Instruction *> &instructionMap, bool backward) {
    std::string instructionName = getInstructionString(I);
    Value *op1 = I.getOperand(0);
    Value *op2 = I.getOperand(1);

    if (!backward) {
        if (isa<llvm::ConstantInt>(op2)) {
            auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
            std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
            interval interval_op1 = blockMap[instructionMap[op1_str]];
            blockMap[instructionMap[instructionName]] = addOp(interval_op1, interval(op2_val, op2_val));
        } else if (isa<llvm::ConstantInt>(op1)) {
            auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
            std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
            interval interval_op2 = blockMap[instructionMap[op2_str]];
            blockMap[instructionMap[instructionName]] = addOp(interval_op2, interval(op1_val, op1_val));
        } else {
            interval interval_op1 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))]];
            interval interval_op2 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op2))]];
            blockMap[instructionMap[instructionName]] = addOp(interval_op1, interval_op2);
        }
    } else {
        auto instructionInterval = blockMap[instructionMap[instructionName]];
        if (isa<llvm::ConstantInt>(op2)) {
            auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
            std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
            blockMap[instructionMap[op1_str]] = subOp(instructionInterval, interval(op2_val, op2_val));
        } else if (isa<llvm::ConstantInt>(op1)) {
            auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
            std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
            blockMap[instructionMap[op2_str]] = subOp(instructionInterval, interval(op1_val, op1_val));
        } else {
            int lowerInstruction = instructionInterval.getMinVal();
            int upperInstruction = instructionInterval.getMaxVal();
            std::string op1String = getInstructionString(*dyn_cast<Instruction>(op1));
            auto op1Interval = blockMap[instructionMap[op1String]];
            std::string op2String = getInstructionString(*dyn_cast<Instruction>(op2));
            auto op2Interval = blockMap[instructionMap[op2String]];
            int lowerOp1 = op1Interval.getMinVal();
            int upperOp1 = op1Interval.getMaxVal();
            int lowerOp2 = op2Interval.getMinVal();
            int upperOp2 = op2Interval.getMaxVal();
            int newLowerOp1 = std::max(lowerOp1, lowerInstruction - upperOp2);
            int newUpperOp1 = std::min(upperOp1, upperInstruction - lowerOp2);
            int newLowerOp2 = std::max(lowerOp2, lowerInstruction - upperOp1);
            int newUpperOp2 = std::min(upperOp2, upperInstruction - lowerOp1);
            blockMap[instructionMap[op1String]] = interval(newLowerOp1, newUpperOp1);
            blockMap[instructionMap[op2String]] = interval(newLowerOp2, newUpperOp2);
        }
    }
}

void analyzeSub(Instruction &I, std::map<Instruction *, interval> &blockMap,
                std::map<std::string, Instruction *> &instructionMap, bool backward) {
    std::string instructionName = getInstructionString(I);
    Value *op1 = I.getOperand(0);
    Value *op2 = I.getOperand(1);

    if (!backward) {
        if (isa<llvm::ConstantInt>(op2)) {
            auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
            interval interval_op1 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))]];
            blockMap[instructionMap[instructionName]] = subOp(interval_op1, interval(op2_val, op2_val));
        } else if (isa<llvm::ConstantInt>(op1)) {
            auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
            interval interval_op2 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op2))]];
            blockMap[instructionMap[instructionName]] = subOp(interval(op1_val, op1_val), interval_op2);
        } else {
            interval interval_op1 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))]];
            interval interval_op2 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op2))]];
            blockMap[instructionMap[instructionName]] = subOp(interval_op1, interval_op2);
        }
    } else {
        auto instructionInterval = blockMap[instructionMap[instructionName]];
        if (isa<llvm::ConstantInt>(op2)) {
            auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
            std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
            blockMap[instructionMap[op1_str]] = addOp(instructionInterval, interval(op2_val, op2_val));
        } else if (isa<llvm::ConstantInt>(op1)) {
            auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
            std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
            blockMap[instructionMap[op2_str]] = subOp(interval(op1_val, op1_val), instructionInterval);
        } else {
            std::string op1String = getInstructionString(*dyn_cast<Instruction>(op1));
            auto op1Interval = blockMap[instructionMap[op1String]];
            std::string op2String = getInstructionString(*dyn_cast<Instruction>(op2));
            auto op2Interval = blockMap[instructionMap[op2String]];
            interval op1Temp = subOp(instructionInterval, op2Interval);
            interval op2Temp = subOp(instructionInterval, op1Interval);
            int newLowerOp1 = std::max(op1Interval.getMinVal(), op1Temp.getMinVal());
            int newUpperOp1 = std::min(op1Interval.getMaxVal(), op1Temp.getMaxVal());
            int newLowerOp2 = std::max(op2Interval.getMinVal(), op2Temp.getMinVal());
            int newUpperOp2 = std::min(op2Interval.getMaxVal(), op2Temp.getMaxVal());
            blockMap[instructionMap[op1String]] = interval(newLowerOp1, newUpperOp1);
            blockMap[instructionMap[op2String]] = interval(newLowerOp2, newUpperOp2);
        }
    }
}

void analyzeMul(Instruction &I, std::map<Instruction *, interval> &blockMap,
                std::map<std::string, Instruction *> &instructionMap, bool backward) {
    std::string instructionName = getInstructionString(I);
    Value *op1 = I.getOperand(0);
    Value *op2 = I.getOperand(1);

    if (!backward) {
        if (isa<llvm::ConstantInt>(op2)) {
            auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
            std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
            interval interval_op1 = blockMap[instructionMap[op1_str]];
            blockMap[instructionMap[instructionName]] = mulOp(interval_op1, interval(op2_val, op2_val));
        } else if (isa<llvm::ConstantInt>(op1)) {
            auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
            std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
            interval interval_op2 = blockMap[instructionMap[op2_str]];
            blockMap[instructionMap[instructionName]] = mulOp(interval_op2, interval(op1_val, op1_val));
        } else {
            interval interval_op1 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))]];
            interval interval_op2 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op2))]];
            blockMap[instructionMap[instructionName]] = mulOp(interval_op1, interval_op2);
        }
    } else {
        auto instructionInterval = blockMap[instructionMap[instructionName]];
        if (isa<llvm::ConstantInt>(op2)) {
            auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
            std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
            blockMap[instructionMap[op1_str]] = divOp(instructionInterval, interval(op2_val, op2_val));
        } else if (isa<llvm::ConstantInt>(op1)) {
            auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
            std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
            blockMap[instructionMap[op2_str]] = divOp(instructionInterval, interval(op1_val, op1_val));
        } else {
            std::string op1String = getInstructionString(*dyn_cast<Instruction>(op1));
            auto op1Interval = blockMap[instructionMap[op1String]];
            std::string op2String = getInstructionString(*dyn_cast<Instruction>(op2));
            auto op2Interval = blockMap[instructionMap[op2String]];
            interval op1Temp = divOp(instructionInterval, op2Interval);
            interval op2Temp = divOp(instructionInterval, op1Interval);
            int newLowerOp1 = std::max(op1Interval.getMinVal(), op1Temp.getMinVal());
            int newUpperOp1 = std::min(op2Interval.getMaxVal(), op1Temp.getMaxVal());
            int newLowerOp2 = std::max(op2Interval.getMinVal(), op2Temp.getMinVal());
            int newUpperOp2 = std::min(op2Interval.getMaxVal(), op2Temp.getMaxVal());
            blockMap[instructionMap[op1String]] = interval(newLowerOp1, newUpperOp1);
            blockMap[instructionMap[op2String]] = interval(newLowerOp2, newUpperOp2);
        }
    }
}

void analyzeSrem(Instruction &I, std::map<Instruction *, interval> &blockMap,
                 std::map<std::string, Instruction *> &instructionMap, bool backward) {
    std::string instructionName = getInstructionString(I);
    Value *op1 = I.getOperand(0);
    Value *op2 = I.getOperand(1);

    if (!backward) {
        if (isa<llvm::ConstantInt>(op2)) {
            auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
            interval interval_op1 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))]];
            blockMap[instructionMap[instructionName]] = sremOp(interval_op1, interval(op2_val, op2_val));
        } else if (isa<llvm::ConstantInt>(op1)) {
            auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
            interval interval_op2 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op2))]];
            blockMap[instructionMap[instructionName]] = sremOp(interval(op1_val, op1_val), interval_op2);
        } else {
            interval interval_op1 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))]];
            interval interval_op2 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op2))]];
            blockMap[instructionMap[instructionName]] = sremOp(interval_op1, interval_op2);
        }
    }
}

void analyzeStore(Instruction &I, std::map<Instruction *, interval> &blockMap,
                  std::map<std::string, Instruction *> &instructionMap, bool backward) {
    Value *op1 = I.getOperand(0);
    Value *op2 = I.getOperand(1);
    if (!backward) {
        if (isa<llvm::ConstantInt>(op1)) {
            auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
            std::string op2Str = getInstructionString(*dyn_cast<Instruction>(op2));
            blockMap[instructionMap[op2Str]] = interval(op1_val, op1_val);
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

void analyzeLoad(Instruction &I, std::map<Instruction *, interval> &blockMap,
                 std::map<std::string, Instruction *> &instructionMap, bool backward) {
    if (!backward) {
        auto *op_instruction = dyn_cast<Instruction>(I.getOperand(0));
        std::string str = getInstructionString(*op_instruction);
        blockMap[instructionMap[getInstructionString(I)]] = blockMap[instructionMap[str]];
    } else {
        auto *op_instruction = dyn_cast<Instruction>(I.getOperand(0));
        std::string str = getInstructionString(*op_instruction);
        if (blockMap[instructionMap[str]].isEmptyInterval()) { ;
        } else if (blockMap[instructionMap[getInstructionString(I)]].isEmptyInterval()) {
            blockMap[instructionMap[str]].setMinVal(POS_LIMIT);
            blockMap[instructionMap[str]].setMaxVal(NEG_LIMIT);
        } else {
            int lower = std::max(blockMap[instructionMap[str]].getMinVal(),
                                 blockMap[instructionMap[getInstructionString(I)]].getMinVal());
            int upper = std::min(blockMap[instructionMap[str]].getMaxVal(),
                                 blockMap[instructionMap[getInstructionString(I)]].getMaxVal());
            blockMap[instructionMap[str]].setMinVal(lower);
            blockMap[instructionMap[str]].setMaxVal(upper);
        }
    }
}

void analyzeAlloca(Instruction &I, std::map<Instruction *, interval> &blockMap,
                   std::map<std::string, Instruction *> &instructionMap, bool backward) {
    if (!backward) {
        blockMap[instructionMap[getInstructionString(I)]] = interval(NEG_LIMIT, POS_LIMIT);
    } else {
        return;
    }
}

void comp(interval &intervalX, interval &intervalY, interval &intervalR) {
    interval xRange = addOp(intervalY, intervalR);
    intervalX.setMinVal(std::max(xRange.getMinVal(), intervalX.getMinVal()));
    intervalX.setMaxVal(std::min(xRange.getMaxVal(), intervalX.getMaxVal()));
    if (intervalX.getMinVal() > intervalX.getMaxVal()) {
        intervalX.setMinVal(POS_LIMIT);
        intervalX.setMaxVal(NEG_LIMIT);
    }
    interval yMinusRange = addOp(intervalR, intervalX);
    intervalY.setMinVal(std::max(yMinusRange.getMaxVal() * -1, intervalY.getMinVal()));
    intervalY.setMaxVal(std::min(yMinusRange.getMinVal() * -1, intervalY.getMaxVal()));
    if (intervalY.getMinVal() > intervalY.getMaxVal()) {
        intervalY.setMinVal(POS_LIMIT);
        intervalY.setMaxVal(NEG_LIMIT);
    }
}

void compare(Value *cmpOp1, Value *cmpOp2, interval &R, std::map<Instruction *, interval> &result,
             std::map<std::string, Instruction *> &instructionMap) {
    if (isa<llvm::ConstantInt>(cmpOp1)) {
        auto op1_val = dyn_cast<llvm::ConstantInt>(cmpOp1)->getZExtValue();
        interval X(op1_val, op1_val);
        comp(X, result[instructionMap[getInstructionString(*dyn_cast<Instruction>(cmpOp2))]], R);
    } else if (isa<llvm::ConstantInt>(cmpOp2)) {
        auto op2_val = dyn_cast<llvm::ConstantInt>(cmpOp2)->getZExtValue();
        interval Y(op2_val, op2_val);
        comp(result[instructionMap[getInstructionString(*dyn_cast<Instruction>(cmpOp1))]], Y, R);
    } else {
        comp(result[instructionMap[getInstructionString(*dyn_cast<Instruction>(cmpOp1))]],
             result[instructionMap[getInstructionString(*dyn_cast<Instruction>(cmpOp2))]], R);
    }
}

void analyzeBr(BasicBlock *BB, Instruction &I, std::map<Instruction *, interval> &blockMap,
               std::map<std::string, Instruction *> &instructionMap,
               std::map<BasicBlock *, std::map<Instruction *, interval>> &result) {
    auto *branchInst = dyn_cast<BranchInst>(&I);
    auto *bb_op1 = branchInst->getSuccessor(0);
    if (!(branchInst->isConditional())) {
        result[bb_op1] = blockMap;
        return;
    }

    auto *cmpIns = dyn_cast<ICmpInst>(I.getOperand(0));

    Value *cmpOp1 = cmpIns->getOperand(0);
    Value *cmpOp2 = cmpIns->getOperand(1);

    auto *bb_op2 = dyn_cast<BasicBlock>(branchInst->getSuccessor(1));

    std::map<Instruction *, interval> branch1 = blockMap;
    std::map<Instruction *, interval> branch2 = blockMap;

    interval Y(NEG_LIMIT, POS_LIMIT);
    switch (cmpIns->getSignedPredicate()) {
        case CmpInst::Predicate::ICMP_SGT : {
            interval R1(1, POS_LIMIT);
            interval R2(NEG_LIMIT, 0);
            compare(cmpOp1, cmpOp2, R1, branch1, instructionMap);
            compare(cmpOp1, cmpOp2, R2, branch2, instructionMap);
            break;
        }
        case CmpInst::Predicate::ICMP_SGE : {
            interval R1(0, POS_LIMIT);
            interval R2(NEG_LIMIT, -1);
            compare(cmpOp1, cmpOp2, R1, branch1, instructionMap);
            compare(cmpOp1, cmpOp2, R2, branch2, instructionMap);
            break;
        }
        case CmpInst::Predicate::ICMP_SLT : {
            interval R1(NEG_LIMIT, -1);
            interval R2(0, POS_LIMIT);
            compare(cmpOp1, cmpOp2, R1, branch1, instructionMap);
            compare(cmpOp1, cmpOp2, R2, branch2, instructionMap);
            break;
        }
        case CmpInst::Predicate::ICMP_SLE : {
            interval R1(NEG_LIMIT, 0);
            interval R2(1, POS_LIMIT);
            compare(cmpOp1, cmpOp2, R1, branch1, instructionMap);
            compare(cmpOp1, cmpOp2, R2, branch2, instructionMap);
            break;
        }
        case CmpInst::Predicate::ICMP_EQ : {
            interval R1(0, 0);
            interval R2(NEG_LIMIT, POS_LIMIT);
            compare(cmpOp1, cmpOp2, R1, branch1, instructionMap);
            compare(cmpOp1, cmpOp2, R2, branch2, instructionMap);
            break;
        }
        case CmpInst::Predicate::ICMP_NE : {
            interval R1(NEG_LIMIT, POS_LIMIT);
            interval R2(0, 0);
            compare(cmpOp1, cmpOp2, R1, branch1, instructionMap);
            compare(cmpOp1, cmpOp2, R2, branch2, instructionMap);
            break;
        }
        default: {
            break;
        }
    }
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
    result[bb_op1] = branch1;
    result[bb_op2] = branch2;
}