#include "ast_term_sorter.h"
#include "ast_sortedness_checker.h"

#include "ast/ast_logic.h"
#include "ast/ast_script.h"
#include "ast/ast_theory.h"
#include "parser/smtlib_parser.h"
#include "util/error_messages.h"
#include "util/global_values.h"

using namespace std;
using namespace smtlib;
using namespace smtlib::ast;

template<class T>
vector<string> toStringArray(vector<shared_ptr<T>>& array) {
    vector<string> strArray;
    for (const auto& node : array) {
        strArray.push_back(node->toString());
    }

    return strArray;
}

void TermSorter::visit(const SimpleIdentifierPtr& node) {
    // Check if it is a variable
    VarInfoPtr varInfo = ctx->getStack()->getVarInfo(node->toString());
    if (varInfo) {
        ret = varInfo->sort;
        return;
    }

    // Check if it is a function
    vector<FunInfoPtr> infos = ctx->getStack()->getFunInfo(node->toString());
    vector<SortPtr> retSorts = extractReturnSorts(infos, 0, false);

    if (retSorts.size() == 1) {
        ret = retSorts[0];
    } else if (retSorts.empty()) {
        auto error = ErrorMessages::buildConstNoSorts(node->toString());
        ctx->getChecker()->addError(error, node);
    } else {
        auto error = ErrorMessages::buildConstMultipleSorts(node->toString(), retSorts);
        ctx->getChecker()->addError(error, node);
    }
}

void TermSorter::visit(const QualifiedIdentifierPtr& node) {
    SortednessChecker::NodeErrorPtr errorAccum;
    errorAccum = ctx->getChecker()->checkSort(node->sort, node, errorAccum);

    SortPtr sortExpanded = ctx->getStack()->expand(node->sort);
    string sortStr = sortExpanded->toString();

    string name = node->identifier->toString();
    vector<FunInfoPtr> infos = ctx->getStack()->getFunInfo(name);

    // Possible non-parametric return sorts
    vector<SortPtr> retSorts = extractReturnSorts(infos, 0, false);

    // Possible parametric return sorts that match
    extractParamMatches(infos, 0, sortExpanded, ctx->getStack(), retSorts);

    // Check if indicated sort is possible
    auto pos = find_if(retSorts.begin(), retSorts.end(),
                       [&](const SortPtr& s) { return s->toString() == sortExpanded->toString(); });

    if (pos != retSorts.end()) {
        ret = *pos;
    } else if (retSorts.empty()) {
        auto error = ErrorMessages::buildConstUnknown(name);
        errorAccum = ctx->getChecker()->addError(error, node, errorAccum);
    } else {
        auto error = ErrorMessages::buildConstWrongSort(name, sortExpanded->toString(), retSorts);
        ctx->getChecker()->addError(error, node, errorAccum);
    }
}

void TermSorter::visit(const DecimalLiteralPtr& node) {
    // Get sort for this type of constant
    vector<FunInfoPtr> infos = ctx->getStack()->getFunInfo(MSCONST_DECIMAL);
    if (infos.size() == 1) {
        if (infos[0]->signature.size() == 1) {
            ret = infos[0]->signature[0];
        }
    } else {
        // If no entries or multiple entries are found, add error
        if (infos.empty()) {
            auto error = ErrorMessages::buildLiteralUnknownSort(MSCONST_DECIMAL_REF);
            ctx->getChecker()->addError(error, node);
        } else {
            vector<SortPtr> possibleSorts = extractReturnSorts(infos, 0, false);
            auto error = ErrorMessages::buildLiteralMultipleSorts(MSCONST_DECIMAL_REF, possibleSorts);
            ctx->getChecker()->addError(error, node);
        }
    }
}

void TermSorter::visit(const NumeralLiteralPtr& node) {
    // Get sort for this type of constant
    vector<FunInfoPtr> infos = ctx->getStack()->getFunInfo(MSCONST_NUMERAL);
    if (infos.size() == 1) {
        if (infos[0]->signature.size() == 1) {
            ret = infos[0]->signature[0];
        }
    } else {
        // If no entries or multiple entries are found, add error
        if (infos.empty()) {
            auto error = ErrorMessages::buildLiteralUnknownSort(MSCONST_NUMERAL_REF);
            ctx->getChecker()->addError(error, node);
        } else {
            vector<SortPtr> possibleSorts = extractReturnSorts(infos, 0, false);
            auto error = ErrorMessages::buildLiteralMultipleSorts(MSCONST_NUMERAL_REF, possibleSorts);
            ctx->getChecker()->addError(error, node);
        }
    }
}

void TermSorter::visit(const StringLiteralPtr& node) {
    // Get sort for this type of constant
    vector<FunInfoPtr> infos = ctx->getStack()->getFunInfo(MSCONST_STRING);
    if (infos.size() == 1) {
        if (infos[0]->signature.size() == 1) {
            ret = infos[0]->signature[0];
        }
    } else {
        // If no entries or multiple entries are found, add error
        if (infos.empty()) {
            auto error = ErrorMessages::buildLiteralUnknownSort(MSCONST_STRING_REF);
            ctx->getChecker()->addError(error, node);
        } else {
            vector<SortPtr> possibleSorts = extractReturnSorts(infos, 0, false);
            auto error = ErrorMessages::buildLiteralMultipleSorts(MSCONST_STRING_REF, possibleSorts);
            ctx->getChecker()->addError(error, node);
        }
    }
}

void TermSorter::visit(const QualifiedTermPtr& node) {
    SortednessChecker::NodeErrorPtr errAccum;

    // Get sorts for arguments
    vector<SortPtr> argSorts;
    vector<TermPtr>& terms = node->terms;
    for (const auto& arg : terms) {
        SortPtr argSort = wrappedVisit(arg);
        if (!argSort) { return; }
        argSorts.push_back(argSort);
    }

    SimpleIdentifierPtr id = dynamic_pointer_cast<SimpleIdentifier>(node->identifier);
    QualifiedIdentifierPtr qid = dynamic_pointer_cast<QualifiedIdentifier>(node->identifier);

    SortPtr retExpanded;
    string name;

    if (id) {
        name = id->toString();
    } else {
        errAccum = ctx->getChecker()->checkSort(qid->sort, node, errAccum);
        name = qid->identifier->toString();
        retExpanded = ctx->getStack()->expand(qid->sort);
    }

    vector<FunInfoPtr> infos = ctx->getStack()->getFunInfo(name);
    vector<SortPtr> retSorts;

    for (const auto& info : infos) {
        // Get function signature, while accounting for all possible attributes (e.g. associativity)
        vector<SortPtr> funSig;
        if (argSorts.size() >= 2) {
            if (info->assocL) {
                funSig.push_back(info->signature[0]);
                for (size_t i = 0; i < argSorts.size() - 1; i++) {
                    funSig.push_back(info->signature[1]);
                }
                funSig.push_back(info->signature[2]);
            } else if (info->assocR) {
                for (size_t i = 0; i < argSorts.size() - 1; i++) {
                    funSig.push_back(info->signature[0]);
                }
                funSig.push_back(info->signature[1]);
                funSig.push_back(info->signature[2]);
            } else if (info->chainable || info->pairwise) {
                for (size_t i = 0; i < argSorts.size(); i++) {
                    funSig.push_back(info->signature[0]);
                }
                funSig.push_back(info->signature[2]);
            } else {
                funSig.insert(funSig.begin(), info->signature.begin(), info->signature.end());
            }
        } else {
            funSig.insert(funSig.begin(), info->signature.begin(), info->signature.end());
        }

        // Check that the arguments respect the signature
        if (argSorts.size() != funSig.size() - 1) { continue; }
        bool fits = true;
        if (info->params.empty()) { // Function is not parametric
            for (size_t i = 0; i < funSig.size() - 1; i++) {
                if (funSig[i]->toString() != argSorts[i]->toString())
                    fits = false;
            }

            if(!fits) { continue; }

            if (id) {
                retSorts.push_back(funSig[funSig.size() - 1]);
            } else {
                SortPtr retSort = funSig[funSig.size() - 1];
                if (retSort->toString() == retExpanded->toString()) {
                    ret = retSort;
                    return;
                }
            }
        } else { // Function is parametric
            vector<string> pnames = toStringArray(info->params);
            unordered_map<string, SortPtr> mapping;

            // Unify each argument sort with its corresponding signature sort
            for (size_t i = 0; i < funSig.size() - 1; i++) {
                fits = fits && unify(funSig[i], argSorts[i], pnames, mapping);
            }

            if (!fits || mapping.size() != info->params.size()) { continue; }

            SortPtr retSort = funSig[funSig.size() - 1];
            retSort = ctx->getStack()->replace(retSort, mapping);
            if (id) {
                retSorts.push_back(retSort);
            } else if (retSort->toString() == retExpanded->toString()) {
                ret = retSort;
                return;
            }
        }
    }

    vector<string> argSortsStr = toStringArray(argSorts);
    vector<string> retSortsStr = toStringArray(retSorts);

    if (id) {
        if (retSorts.size() == 1) {
            ret = retSorts[0];
        } else if (retSorts.empty()) {
            auto error = ErrorMessages::buildFunUnknownDecl(name, argSortsStr);
            errAccum = ctx->getChecker()->addError(error, node, errAccum);
        } else {
            auto error = ErrorMessages::buildFunMultipleDecls(name, argSortsStr, retSortsStr);
            errAccum = ctx->getChecker()->addError(error, node, errAccum);
        }
    } else {
        auto error = ErrorMessages::buildFunUnknownDecl(name, argSortsStr, retExpanded->toString());
        errAccum = ctx->getChecker()->addError(error, node, errAccum);
    }
}

void TermSorter::visit(const LetTermPtr& node) {
    // New stack level for bindings
    ctx->getStack()->push();

    // Push bindings
    vector<VariableBindingPtr>& bindings = node->bindings;
    for (const auto& bind : bindings) {
        SortPtr bindSort = wrappedVisit(bind->term);
        if (bindSort) {
            ctx->getStack()->tryAdd(make_shared<VarInfo>(std::move(bind->symbol->toString()), bindSort, node));
        } else {
            return;
        }
    }

    // Determine sort of the inner term
    SortPtr termSort = wrappedVisit(node->term);
    if (termSort) {
        ret = termSort;
    }

    // Pop the previously added level
    ctx->getStack()->pop();
}

void TermSorter::visit(const ForallTermPtr& node) {
    // New stack level for bindings
    ctx->getStack()->push();

    // Push bindings
    vector<SortedVariablePtr>& bindings = node->bindings;
    for (const auto& bind : bindings) {
        auto bindSortExpanded = ctx->getStack()->expand(bind->sort);
        ctx->getStack()->tryAdd(make_shared<VarInfo>(std::move(bind->symbol->toString()),
                                                     bindSortExpanded, node));
    }

    // Determine sort of the inner term
    SortPtr termSort = wrappedVisit(node->term);
    if (termSort) {
        string termSortStr = termSort->toString();
        // Inner term should be boolean
        if (termSortStr == SORT_BOOL) {
            ret = termSort;
        } else {
            // Otherwise, add error
            auto error = ErrorMessages::buildQuantTermWrongSort(node->term->toString(), termSortStr, SORT_BOOL,
                                                                node->term->rowLeft, node->term->colLeft,
                                                                node->term->rowRight, node->term->colRight);
            ctx->getChecker()->addError(error, node);
        }
    }

    // Pop the previously added level
    ctx->getStack()->pop();
}

void TermSorter::visit(const ExistsTermPtr& node) {
    // New stack level for bindings
    ctx->getStack()->push();

    // Push bindings
    vector<SortedVariablePtr>& bindings = node->bindings;
    for (const auto& bind : bindings) {
        auto bindSortExpanded = ctx->getStack()->expand(bind->sort);
        ctx->getStack()->tryAdd(make_shared<VarInfo>(std::move(bind->symbol->toString()),
                                                     bindSortExpanded, node));
    }

    // Determine sort of the inner term
    SortPtr termSort = wrappedVisit(node->term);

    if (termSort) {
        string termSortStr = termSort->toString();
        // Inner term should be boolean
        if (termSortStr == SORT_BOOL) {
            ret = termSort;
        } else {
            // Otherwise, add error
            auto error = ErrorMessages::buildQuantTermWrongSort(node->term->toString(), termSortStr, SORT_BOOL,
                                                                node->term->rowLeft, node->term->colLeft,
                                                                node->term->rowRight, node->term->colRight);
            ctx->getChecker()->addError(error, node);
        }
    }

    // Pop the previously added level
    ctx->getStack()->pop();
}

void TermSorter::visit(const MatchTermPtr& node) {
    // Determine sort of the term to be matched
    SortPtr termSort = wrappedVisit(node->term);

    // Return if sort could not be determined
    if (!termSort) {
        return;
    }

    // Determine the sort of each match case
    vector<SortPtr> caseSorts;

    SortednessChecker::NodeErrorPtr errAccum;
    string termSortStr = termSort->toString();

    vector<MatchCasePtr>& cases = node->cases;
    for (size_t i = 0, szi = cases.size(); i < szi; i++) {
        PatternPtr pattern = cases[i]->pattern;

        // Symbol (constructor or variable)
        SymbolPtr spattern = dynamic_pointer_cast<Symbol>(pattern);
        // Qualified constructor
        QualifiedConstructorPtr cpattern = dynamic_pointer_cast<QualifiedConstructor>(pattern);
        // Qualified pattern
        QualifiedPatternPtr qpattern = dynamic_pointer_cast<QualifiedPattern>(pattern);

        SymbolPtr scons; // Simple constructor for qualified pattern
        QualifiedConstructorPtr qcons; // Qualified constructor for qualified pattern
        string caseId;

        // Initialize caseId, scons, qcons
        if (spattern) {
            caseId = spattern->toString();
        } else if (cpattern) {
            caseId = cpattern->symbol->toString();
        } else if (qpattern) {
            ConstructorPtr cons = qpattern->constructor;
            scons = dynamic_pointer_cast<Symbol>(cons);
            qcons = dynamic_pointer_cast<QualifiedConstructor>(cons);

            if (scons)
                caseId = scons->toString();
            else
                caseId = qcons->symbol->toString();
        }

        // Get known entries for functions with the name caseId
        vector<FunInfoPtr> funInfos = ctx->getStack()->getFunInfo(caseId);
        vector<FunInfoPtr> matchingInfos;
        vector<unordered_map<string, SortPtr>> matchingMappings;

        // Select the function entries that fit
        for (const auto& info : funInfos) {
            SortPtr retSort = info->signature[info->signature.size() - 1];
            string retSortStr = retSort->toString();

            // If entry is about a parametric function, map sort parameters to real sorts
            vector<string> pnames = toStringArray(info->params);
            unordered_map<string, SortPtr> mapping;
            bool mapped = pnames.empty() || unify(retSort, termSort, pnames, mapping);

            // Check if current function entry fits
            if (spattern || cpattern) {
                // Return sort mismatch in case of qualified constructor
                if (cpattern && cpattern->sort->toString() != termSortStr) {
                    auto error = ErrorMessages::buildPatternMismatch(termSortStr, pattern->toString());
                    errAccum = ctx->getChecker()->addError(error, node, errAccum);
                    continue;
                }

                // If return sorts were mapped correctly
                if (mapped && info->params.size() == mapping.size()) {
                    matchingInfos.push_back(info);
                    matchingMappings.push_back(mapping);
                }
            } else if (qpattern) {
                // Return sort mismatch in case of qualified constructor
                if (qcons && qcons->sort->toString() != termSortStr) {
                    auto error = ErrorMessages::buildPatternMismatch(termSortStr, pattern->toString());
                    errAccum = ctx->getChecker()->addError(error, node, errAccum);
                    continue;
                }

                // If return sorts were mapped correctly
                // and there are as many arguments to the function as there are symbols in the pattern
                if (mapped && info->params.size() == mapping.size()
                    && qpattern->symbols.size() == info->signature.size() - 1) {
                    matchingInfos.push_back(info);
                    matchingMappings.push_back(mapping);
                }
            }
        }

        if (matchingInfos.empty()) {
            if (spattern && i + 1 >= szi) {
                // If it's not a function, try to interpret it as a variable
                ctx->getStack()->push();
                ctx->getStack()->tryAdd(make_shared<VarInfo>(caseId, termSort, cases[i]));
                SortPtr caseSort = wrappedVisit(cases[i]->term);
                if (caseSort) {
                    caseSorts.push_back(caseSort);
                }
                ctx->getStack()->pop();
            } else if (spattern || cpattern) {
                auto error = ErrorMessages::buildFunUnknownDecl(caseId, termSort->toString());
                errAccum = ctx->getChecker()->addError(error, node, errAccum);
            } else if (qpattern) {
                auto error = ErrorMessages::buildFunUnknownDecl(caseId, qpattern->symbols.size(), termSort->toString());
                errAccum = ctx->getChecker()->addError(error, node, errAccum);
            }
        } else if (matchingInfos.size() > 1) {
            if (qpattern) {
                auto error = ErrorMessages::buildFunMultipleDecls(caseId, qpattern->symbols.size(), termSort->toString());
                errAccum = ctx->getChecker()->addError(error, node, errAccum);
            }
        } else {
            FunInfoPtr match = matchingInfos[0];
            if (qpattern) {
                ctx->getStack()->push();
                for (size_t j = 0, szj = match->signature.size(); j < szj - 1; j++) {
                    SortPtr paramSort = ctx->getStack()->replace(match->signature[j], matchingMappings[0]);
                    ctx->getStack()->tryAdd(make_shared<VarInfo>(std::move(qpattern->symbols[j]->toString()),
                                                                 paramSort, cases[i]));
                }
            }

            SortPtr caseSort = wrappedVisit(cases[i]->term);
            if (caseSort) {
                caseSorts.push_back(caseSort);
            }

            if (qpattern) {
                ctx->getStack()->pop();
            }
        }
    }

    // Check that all cases have the same sort
    if (caseSorts.size() == node->cases.size()) {
        string case1 = caseSorts[0]->toString();
        auto pos = find_if(caseSorts.begin() + 1, caseSorts.end(),
                           [&](const SortPtr& s) { return s->toString() != case1; });

        bool equalCases = pos == caseSorts.end();
        if (equalCases) {
            ret = caseSorts[0];
        } else {
            errAccum = ctx->getChecker()->addError(ErrorMessages::buildCasesMismatch(caseSorts), node, errAccum);
        }
    }

}

void TermSorter::visit(const AnnotatedTermPtr& node) {
    visit0(node->term);
}

vector<SortPtr> TermSorter::extractReturnSorts(const vector<FunInfoPtr>& infos,
                                               size_t arity, bool parametric) {
    vector<SortPtr> retSorts;
    for (const FunInfoPtr& info : infos) {
        size_t sz = info->signature.size();
        if (sz == arity + 1 && !info->params.empty() == parametric)
            retSorts.push_back(info->signature[sz - 1]);
    }

    return retSorts;
}

void TermSorter::extractReturnSorts(const vector<FunInfoPtr>& infos,
                                    size_t arity, bool parametric,
                                    vector<SortPtr>& accum) {
    vector<SortPtr> retSorts = extractReturnSorts(infos, arity, parametric);
    accum.insert(accum.begin(), retSorts.begin(), retSorts.end());
}

vector<string> TermSorter::extractParamNames(const FunInfoPtr& info) {
    vector<string> paramNames;
    for (const auto& param : info->params) {
        paramNames.push_back(param->toString());
    }

    return paramNames;
}

vector<SortPtr> TermSorter::extractParamMatches(const vector<FunInfoPtr>& infos,
                                                     size_t arity, const SortPtr& sort,
                                                     const SymbolStackPtr& stack) {
    vector<SortPtr> matches;

    for (const auto& info : infos) {
        size_t sz = info->signature.size();
        if (sz == arity + 1 && !info->params.empty()) {
            vector<string> paramNames = extractParamNames(info);
            unordered_map<string, SortPtr> mapping;
            unify(info->signature[sz - 1], sort, paramNames, mapping);

            if (mapping.size() == paramNames.size()) {
                SortPtr retSort = stack->replace(info->signature[sz - 1], mapping);
                matches.push_back(retSort);
            }
        }
    }

    return matches;
}

void TermSorter::extractParamMatches(const vector<FunInfoPtr>& infos,
                                     size_t arity, const SortPtr& sort,
                                     const SymbolStackPtr& stack,
                                     vector<SortPtr>& accum) {
    vector<SortPtr> matches = extractParamMatches(infos, arity, sort, stack);
    accum.insert(accum.begin(), matches.begin(), matches.end());
}

bool TermSorter::unify(const SortPtr& sort1, const SortPtr& sort2,
                       const vector<string>& params,
                       unordered_map<string, SortPtr>& mapping) {
    if (!sort1 || !sort2)
        return false;

    string sort1Name = sort1->identifier->toString();
    string sort2Name = sort2->identifier->toString();
    bool isParam = (find(params.begin(), params.end(), sort1Name) != params.end());

    if (isParam) {
        if (mapping[sort1Name]) {
            return mapping[sort1Name]->toString() == sort2->toString();
        } else {
            mapping[sort1Name] = sort2;
            return true;
        }
    } else {
        if (sort1->arguments.size() != sort2->arguments.size()) {
            return false;
        } else {
            if (sort1Name != sort2Name) {
                return false;
            }

            bool fits = true;
            for (unsigned long i = 0; i < sort1->arguments.size(); i++) {
                fits = fits && unify(sort1->arguments[i], sort2->arguments[i], params, mapping);
            }

            return fits;
        }
    }
}
