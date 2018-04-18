#include "ast_symbol_table.h"

#include "ast/ast_command.h"
#include "ast/ast_symbol_decl.h"

using namespace std;
using namespace smtlib::ast;

SortInfoPtr SymbolTable::getSortInfo(const string& name) {
    auto it = sorts.find(name);
    if (it != sorts.end()) {
        return it->second;
    } else {
        SortInfoPtr empty;
        return empty;
    }
}

std::vector<FunInfoPtr> SymbolTable::getFunInfo(const string& name) {
    auto it = funs.find(name);
    if (it != funs.end()) {
        return it->second;
    } else {
        std::vector<FunInfoPtr> empty;
        return empty;
    }
}

VarInfoPtr SymbolTable::getVarInfo(const string& name) {
    auto it = vars.find(name);
    if (it != vars.end()) {
        return it->second;
    } else {
        VarInfoPtr empty;
        return empty;
    }
}

bool SymbolTable::add(const SortInfoPtr& info) {
    if(sorts.find(info->name) == sorts.end()) {
        sorts[info->name] = info;
        return true;
    } else {
        return false;
    }
}

bool SymbolTable::add(const FunInfoPtr& info) {
    funs[info->name].push_back(info);
    return true;
}

bool SymbolTable::add(const VarInfoPtr& info) {
    if(vars.find(info->name) == vars.end()) {
        vars[info->name] = info;
        return true;
    } else {
        return false;
    }
}

void SymbolTable::reset() {
    // Clear all variables
    vars.clear();

    // Erase sort information that does not come from theory files
    std::vector<SortInfoPtr> sortInfos;
    for (const auto& sort : sorts) {
        sortInfos.push_back(sort.second);
    }

    for (const auto& sortInfo : sortInfos) {
        if(!dynamic_pointer_cast<SortSymbolDeclaration>(sortInfo->source)) {
            sorts.erase(sortInfo->name);
        }
    }

    // Erase function information that does not come from theory files
    vector<string> funKeys;
    vector<std::vector<FunInfoPtr>> funInfos;
    for (const auto& fun : funs) {
        funKeys.push_back(fun.first);
        funInfos.push_back(fun.second);
    }

    for (int i = 0; i < funKeys.size(); i++) {
        std::vector<FunInfoPtr>& info = funs[funKeys[i]];
        for (int j = 0; j < funInfos[i].size(); j++) {
            if(!dynamic_pointer_cast<FunSymbolDeclaration>(funInfos[i][j]->source)) {
                info.erase(info.begin() + j);
            }
        }
        if(info.empty())
            funs.erase(funKeys[i]);
    }
}