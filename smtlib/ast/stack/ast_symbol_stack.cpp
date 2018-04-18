#include "ast_symbol_stack.h"

using namespace std;
using namespace smtlib::ast;

SymbolStack::SymbolStack() {
    push();
}

SymbolTablePtr SymbolStack::getTopLevel() {
    return stack[stack.size() - 1];
}

std::vector<SymbolTablePtr>& SymbolStack::getStack() {
    return stack;
}

bool SymbolStack::push() {
    size_t size = stack.size();
    stack.push_back(make_shared<SymbolTable>());
    return (stack.size() == size + 1);
}

bool SymbolStack::push(size_t levels) {
    size_t size = stack.size();
    for (int i = 0; i < levels; i++)
        stack.push_back(make_shared<SymbolTable>());
    return (stack.size() == size + levels);
}

bool SymbolStack::pop() {
    if (stack.size() <= 1) {
        return false;
    } else {
        size_t size = stack.size();
        stack.erase(stack.begin() + (stack.size() - 1));
        return (stack.size() == size - 1);
    }
}

bool SymbolStack::pop(size_t levels) {
    if (stack.size() <= 1 + levels || levels == 0) {
        return false;
    } else {
        size_t size = stack.size();
        stack.erase(stack.begin() + (stack.size() - levels), stack.begin() + (stack.size() - 1));
        return (stack.size() == size - 1);
    }
}

void SymbolStack::reset() {
    pop(stack.size() - 1);
    getTopLevel()->reset();
}

SortInfoPtr SymbolStack::getSortInfo(const string& name) {
    SortInfoPtr null;
    for (const auto& lvl : stack) {
        SortInfoPtr info = lvl->getSortInfo(name);
        if (info)
            return info;
    }
    return null;
}

std::vector<FunInfoPtr> SymbolStack::getFunInfo(const string& name) {
    std::vector<FunInfoPtr> result;
    for (const auto& lvl : stack) {
        std::vector<FunInfoPtr> infos = lvl->getFunInfo(name);
        result.insert(result.end(), infos.begin(), infos.end());
    }
    return result;
}

VarInfoPtr SymbolStack::getVarInfo(const string& name) {
    VarInfoPtr null;
    for (const auto& lvl : stack) {
        VarInfoPtr info = lvl->getVarInfo(name);
        if (info)
            return info;
    }
    return null;
}

SortInfoPtr SymbolStack::findDuplicate(const SortInfoPtr& info) {
    SortInfoPtr null;
    for (const auto& lvl : stack) {
        SortInfoPtr dup = lvl->getSortInfo(info->name);
        if (dup)
            return dup;
    }
    return null;
}

FunInfoPtr SymbolStack::findDuplicate(const FunInfoPtr& info) {
    FunInfoPtr null;
    std::vector<FunInfoPtr> knownFuns = getFunInfo(info->name);
    for (const auto& fun : knownFuns) {
        if (info->params.empty() && fun->params.empty()) {
            if (equal(info->signature, fun->signature)) {
                return fun;
            }
        } else {
            if (equal(info->params, info->signature, fun->params, fun->signature)) {
                return fun;
            }
        }
    }
    return null;
}

VarInfoPtr SymbolStack::findDuplicate(const VarInfoPtr& info) {
    return getTopLevel()->getVarInfo(info->name);
}

SortPtr SymbolStack::replace(const SortPtr& sort,
                             unordered_map<string, SortPtr>& mapping) {
    if (mapping.empty())
        return sort;

    if (!sort->hasArgs()) {
        if (mapping.find(sort->toString()) != mapping.end())
            return mapping[sort->toString()];
        else
            return sort;
    } else {
        std::vector<SortPtr> newargs;
        bool changed = false;
        std::vector<SortPtr>& argSorts = sort->arguments;
        for (const auto& argSort : argSorts) {
            SortPtr result = replace(argSort, mapping);

            newargs.push_back(result);
            if (result.get() != argSort.get())
                changed = true;
        }

        if (changed) {
            return make_shared<Sort>(sort->identifier, newargs);
        } else {
            return sort;
        }
    }
}

SortPtr SymbolStack::expand(const SortPtr& sort) {
    if (!sort)
        return sort;

    SortPtr null;

    SortInfoPtr info = getSortInfo(sort->identifier->toString());
    if (!sort->hasArgs()) {
        if (info && info->definition) {
            if (info->definition->params.empty()) {
                SortPtr newsort = make_shared<Sort>(info->definition->sort->identifier,
                                                    info->definition->sort->arguments);
                newsort->rowLeft = sort->rowLeft;
                newsort->colLeft = sort->colLeft;
                newsort->rowRight = sort->rowRight;
                newsort->colRight = sort->colRight;
                newsort->filename = sort->filename;

                return newsort;
            } else {
                return null;
            }
        } else {
            return sort;
        }
    } else {
        if (info && info->definition) {
            if (info->definition->params.size() == sort->arguments.size()) {
                unordered_map<string, SortPtr> mapping;
                for (int i = 0; i < info->definition->params.size(); i++) {
                    mapping[info->definition->params[i]->toString()] = sort->arguments[i];
                }

                SortPtr newsort = replace(info->definition->sort, mapping);
                newsort = expand(newsort);
                newsort->rowLeft = sort->rowLeft;
                newsort->colLeft = sort->colLeft;
                newsort->rowRight = sort->rowRight;
                newsort->colRight = sort->colRight;
                newsort->filename = sort->filename;

                return newsort;
            } else {
                return null;
            }
        } else {
            if (info && info->arity != sort->arguments.size())
                return null;

            std::vector<SortPtr> newargs;
            bool changed = false;
            std::vector<SortPtr>& argSorts = sort->arguments;
            for (const auto& arg : argSorts) {
                SortPtr result = expand(arg);
                if (!result)
                    return null;

                newargs.push_back(result);
                if (result.get() != arg.get())
                    changed = true;
            }

            if (changed) {
                SortPtr newsort = make_shared<Sort>(sort->identifier, newargs);
                newsort->rowLeft = sort->rowLeft;
                newsort->colLeft = sort->colLeft;
                newsort->rowRight = sort->rowRight;
                newsort->colRight = sort->colRight;
                newsort->filename = sort->filename;

                return newsort;
            } else {
                return sort;
            }
        }
    }
}

bool SymbolStack::equal(const SortPtr& sort1, const SortPtr& sort2) {
    if (sort1 && sort2) {
        return sort1->toString() == sort2->toString();
    } else {
        return false;
    }
}

bool SymbolStack::equal(const std::vector<SymbolPtr>& params1, const SortPtr& sort1,
                        const std::vector<SymbolPtr>& params2, const SortPtr& sort2,
                        unordered_map<string, string>& mapping) {
    if (!sort1 || !sort2) {
        return false;
    }

    if (sort1->arguments.size() != sort2->arguments.size())
        return false;

    if (sort1->arguments.empty()) {
        bool isParam1 = false;
        bool isParam2 = false;

        string str1 = sort1->toString();
        string str2 = sort2->toString();

        for (size_t j = 0, sz = params1.size(); j < sz; j++) {
            if (params1[j]->toString() == str1)
                isParam1 = true;
            if (params2[j]->toString() == str2)
                isParam2 = true;
        }

        if ((isParam1 && !isParam2) || (!isParam1 && isParam2)) {
            return false;
        } else if (isParam1) {
            if (mapping.find(str1) != mapping.end()) {
                return mapping[str1] == str2;
            } else {
                mapping[str1] = str2;
                return true;
            }
        } else {
            return equal(sort1, sort2);
        }
    } else {
        if (sort1->identifier->toString() != sort2->identifier->toString())
            return false;

        for (size_t k = 0, sz = sort1->arguments.size(); k < sz; k++) {
            if (!equal(params1, sort1->arguments[k], params2, sort2->arguments[k], mapping))
                return false;
        }

        return true;
    }
}

bool SymbolStack::equal(const std::vector<SortPtr>& signature1,
                        const std::vector<SortPtr>& signature2) {
    if (signature1.size() != signature2.size())
        return false;

    for (size_t i = 0, sz = signature1.size(); i < sz; i++) {
        if (!equal(signature1[i], signature2[i]))
            return false;
    }

    return true;
}

bool SymbolStack::equal(const std::vector<SymbolPtr>& params1,
                        const std::vector<SortPtr>& signature1,
                        const std::vector<SymbolPtr>& params2,
                        const std::vector<SortPtr>& signature2) {
    if (params1.size() != params2.size() || signature1.size() != signature2.size())
        return false;

    unordered_map<string, string> mapping;
    for (size_t i = 0, sz = signature1.size(); i < sz; i++) {
        SortPtr sort1 = signature1[i];
        SortPtr sort2 = signature2[i];

        if (!equal(params1, sort1, params2, sort2, mapping))
            return false;
    }

    return mapping.size() == params1.size();
}

SortInfoPtr SymbolStack::tryAdd(const SortInfoPtr& info) {
    SortInfoPtr dup = findDuplicate(info);
    if (!dup)
        getTopLevel()->add(info);
    return dup;
}

FunInfoPtr SymbolStack::tryAdd(const FunInfoPtr& info) {
    FunInfoPtr dup = findDuplicate(info);
    if (!dup)
        getTopLevel()->add(info);
    return dup;
}

VarInfoPtr SymbolStack::tryAdd(const VarInfoPtr& info) {
    VarInfoPtr dup = findDuplicate(info);
    if (!dup)
        getTopLevel()->add(info);
    return dup;
}