#include "ast_sortedness_checker.h"
#include "ast_syntax_checker.h"
#include "ast/ast_logic.h"
#include "ast/ast_script.h"
#include "ast/ast_theory.h"
#include "parser/smtlib_parser.h"
#include "util/error_messages.h"
#include "util/global_values.h"
#include "../../../exec/execution.h"

using namespace std;
using namespace slcompparser;
using namespace smtlib;
using namespace smtlib::ast;

/* ================================ SortednessChecker ================================= */

SortednessChecker::NodeErrorPtr SortednessChecker::addError(const string& message,
                                                            const NodePtr& node,
                                                            NodeErrorPtr& err) {
    if (!err) {
        err = std::move(make_shared<NodeError>(std::move(make_shared<Error>(message)), node));

        if (node && node->filename)
            errors[*(node->filename)].push_back(err);
        else
            errors[""].push_back(err);
    } else {
        err->errs.push_back(make_shared<Error>(message));
    }

    return err;
}

SortednessChecker::NodeErrorPtr SortednessChecker::addError(const string& message,
                                                            const NodePtr& node,
                                                            const SymbolInfoPtr& info,
                                                            SortednessChecker::NodeErrorPtr& err) {
    if (!err) {
        err = std::move(make_shared<NodeError>(std::move(make_shared<Error>(message, info)), node));
        if (node && node->filename)
            errors[*(node->filename)].push_back(err);
        else
            errors[""].push_back(err);
    } else {
        err->errs.push_back(make_shared<Error>(message, info));
    }

    return err;
}

void SortednessChecker::addError(const string& message, const NodePtr& node) {
    NodeErrorPtr err = std::move(make_shared<NodeError>(std::move(make_shared<Error>(message)), node));
    if (node && node->filename)
        errors[*(node->filename)].push_back(std::move(err));
    else
        errors[""].push_back(std::move(err));
}

void SortednessChecker::addError(const string& message, const NodePtr& node,
                                 const SymbolInfoPtr& info) {
    NodeErrorPtr err = std::move(make_shared<NodeError>(std::move(make_shared<Error>(message, info)), node));
    errors[*(node->filename)].push_back(std::move(err));
}

SortInfoPtr SortednessChecker::getInfo(const DeclareSortCommandPtr& node) {
    return make_shared<SortInfo>(std::move(node->symbol->toString()),
                                 node->arity->value, node);
}

SortInfoPtr SortednessChecker::getInfo(const DefineSortCommandPtr& node) {
    return make_shared<SortInfo>(std::move(node->symbol->toString()),
                                 node->parameters.size(),
                                 node->parameters,
                                 ctx->getStack()->expand(node->sort), node);
}


SortInfoPtr SortednessChecker::getInfo(const SortSymbolDeclarationPtr& node) {
    return make_shared<SortInfo>(std::move(node->identifier->toString()),
                                 node->arity->value,
                                 node->attributes, node);
}

FunInfoPtr SortednessChecker::getInfo(const SpecConstFunDeclarationPtr& node) {
    std::vector<SortPtr> sig;
    sig.push_back(ctx->getStack()->expand(node->sort));
    return make_shared<FunInfo>(std::move(node->constant->toString()),
                                sig, node->attributes, node);
}

FunInfoPtr SortednessChecker::getInfo(const MetaSpecConstFunDeclarationPtr& node) {
    std::vector<SortPtr> sig;
    sig.push_back(ctx->getStack()->expand(node->sort));
    return make_shared<FunInfo>(std::move(node->constant->toString()),
                                sig, node->attributes, node);
}

FunInfoPtr SortednessChecker::getInfo(const SimpleFunDeclarationPtr& node) {
    std::vector<SortPtr> newsig;
    for (const auto& sort : node->signature) {
        newsig.push_back(ctx->getStack()->expand(sort));
    }

    FunInfoPtr funInfo = make_shared<FunInfo>(std::move(node->identifier->toString()),
                                              newsig, node->attributes, node);
    for (const auto& attr : node->attributes) {
        string attrString = attr->toString();
        if (attrString == KW_RIGHT_ASSOC) {
            funInfo->assocR = true;
        }

        if (attrString == KW_LEFT_ASSOC) {
            funInfo->assocL = true;
        }

        if (attrString == KW_CHAINABLE) {
            funInfo->chainable = true;
        }

        if (attrString == KW_PAIRWISE) {
            funInfo->pairwise = true;
        }
    }

    return funInfo;
}

FunInfoPtr SortednessChecker::getInfo(const ParametricFunDeclarationPtr& node) {
    std::vector<SortPtr> newsig;
    for (const auto& sort : node->signature) {
        newsig.push_back(ctx->getStack()->expand(sort));
    }

    FunInfoPtr funInfo = make_shared<FunInfo>(std::move(node->identifier->toString()), newsig,
                                              node->parameters, node->attributes, node);

    for (const auto& attr : node->attributes) {
        string attrString = attr->toString();
        if (attrString == KW_RIGHT_ASSOC) {
            funInfo->assocR = true;
        }

        if (attrString == KW_LEFT_ASSOC) {
            funInfo->assocL = true;
        }

        if (attrString == KW_CHAINABLE) {
            funInfo->chainable = true;
        }

        if (attrString == KW_PAIRWISE) {
            funInfo->pairwise = true;
        }
    }

    return funInfo;
}

FunInfoPtr SortednessChecker::getInfo(const DeclareConstCommandPtr& node) {
    std::vector<SortPtr> newsig;
    newsig.push_back(ctx->getStack()->expand(node->sort));

    return make_shared<FunInfo>(node->symbol->toString(), newsig, node);
}

FunInfoPtr SortednessChecker::getInfo(const DeclareFunCommandPtr& node) {
    std::vector<SortPtr> newsig;
    for (const auto& param : node->parameters) {
        SortPtr itsort = ctx->getStack()->expand(param);
        newsig.push_back(itsort);
    }
    SortPtr retsort = ctx->getStack()->expand(node->sort);
    newsig.push_back(retsort);

    return make_shared<FunInfo>(node->symbol->toString(), newsig, node);
}

FunInfoPtr SortednessChecker::getInfo(const DefineFunCommandPtr& node) {
    std::vector<SortPtr> newsig;
    for (const auto& param : node->definition->signature->parameters) {
        newsig.push_back(ctx->getStack()->expand(param->sort));
    }
    newsig.push_back(ctx->getStack()->expand(node->definition->signature->sort));

    return make_shared<FunInfo>(node->definition->signature->symbol->toString(),
                                newsig, node->definition->body, node);
}

FunInfoPtr SortednessChecker::getInfo(const DefineFunRecCommandPtr& node) {
    std::vector<SortPtr> newsig;

    for (const auto& param : node->definition->signature->parameters) {
        newsig.push_back(ctx->getStack()->expand(param->sort));
    }
    newsig.push_back(ctx->getStack()->expand(node->definition->signature->sort));

    return make_shared<FunInfo>(std::move(node->definition->signature->symbol->toString()),
                                newsig, node->definition->body, node);
}

std::vector<FunInfoPtr> SortednessChecker::getInfo(const DefineFunsRecCommandPtr& node) {
    std::vector<FunInfoPtr> infos;
    for (size_t i = 0, sz = node->declarations.size(); i < sz; i++) {
        std::vector<SortPtr> newsig;
        for (const auto& param : node->declarations[i]->parameters) {
            newsig.push_back(ctx->getStack()->expand(param->sort));
        }
        newsig.push_back(ctx->getStack()->expand(node->declarations[i]->sort));

        infos.push_back(make_shared<FunInfo>(node->declarations[i]->symbol->toString(),
                                             newsig, node->bodies[i], node));
    }

    return infos;
}

std::vector<SymbolInfoPtr> SortednessChecker::getInfo(const DeclareDatatypeCommandPtr& node) {
    std::vector<SymbolInfoPtr> infos;
    string typeName = node->symbol->toString();

    ParametricDatatypeDeclarationPtr pdecl =
            dynamic_pointer_cast<ParametricDatatypeDeclaration>(node->declaration);

    if (pdecl) {
        // Add datatype (parametric) sort info
        infos.push_back(make_shared<SortInfo>(typeName, pdecl->parameters.size(), node));

        // Build a sort representing the datatype (to be used in the signatures of the constructors and selectors)
        SortPtr typeSort = make_shared<Sort>(std::move(make_shared<SimpleIdentifier>(node->symbol)));

        std::vector<SymbolPtr>& params = pdecl->parameters;
        for (const auto& param : params) {
            typeSort->arguments.push_back(make_shared<Sort>(make_shared<SimpleIdentifier>(param)));
        }

        typeSort = ctx->getStack()->expand(typeSort);

        for (const auto& cons : pdecl->constructors) {
            // Start building function info for current constructor
            string consName = cons->symbol->toString();
            std::vector<SortPtr> consSig;

            for (const auto& sel : cons->selectors) {
                // Add sort of current selector to current constructor signature
                consSig.push_back(ctx->getStack()->expand(sel->sort));

                // Build function info for current selector
                string selName = sel->symbol->toString();
                std::vector<SortPtr> selSig;
                selSig.push_back(typeSort);
                selSig.push_back(ctx->getStack()->expand(sel->sort));

                // Add selector function info
                infos.push_back(make_shared<FunInfo>(selName, selSig, pdecl->parameters, node));
            }

            // Add constructor function info
            consSig.push_back(typeSort);
            infos.push_back(make_shared<FunInfo>(consName, consSig, pdecl->parameters, node));
        }

    } else {
        // Add datatype (non-parametric) sort info
        infos.push_back(make_shared<SortInfo>(typeName, 0, node));

        // Build a sort representing the datatype (to be used in the signatures of the constructors and selectors)
        SortPtr typeSort = make_shared<Sort>(make_shared<SimpleIdentifier>(node->symbol));
        typeSort = ctx->getStack()->expand(typeSort);

        SimpleDatatypeDeclarationPtr sdecl =
                dynamic_pointer_cast<SimpleDatatypeDeclaration>(node->declaration);

        for (const auto& cons : sdecl->constructors) {
            // Start building function info for current constructor
            string consName = cons->symbol->toString();
            std::vector<SortPtr> consSig;

            for (const auto& sel : cons->selectors) {
                // Add sort of current selector to current constructor signature
                consSig.push_back(ctx->getStack()->expand(sel->sort));

                // Build function info for current selector
                string selName = sel->symbol->toString();
                std::vector<SortPtr> selSig;
                selSig.push_back(typeSort);
                selSig.push_back(ctx->getStack()->expand(sel->sort));

                // Add selector function info
                infos.push_back(make_shared<FunInfo>(selName, selSig, node));
            }

            // Add constructor function info
            consSig.push_back(typeSort);
            infos.push_back(make_shared<FunInfo>(consName, consSig, node));
        }
    }

    return infos;
}

std::vector<SymbolInfoPtr> SortednessChecker::getInfo(const DeclareDatatypesCommandPtr& node) {
    std::vector<SymbolInfoPtr> infos;

    std::vector<SortDeclarationPtr>& datatypeSorts = node->sorts;
    for (const auto& sort : datatypeSorts) {
        string typeName = sort->symbol->toString();
        size_t arity = (size_t) sort->arity->value;

        // Add datatype sort info
        infos.push_back(make_shared<SortInfo>(typeName, arity, node));
    }

    for (size_t i = 0, sz = node->sorts.size(); i < sz; i++) {
        ParametricDatatypeDeclarationPtr pdecl =
                dynamic_pointer_cast<ParametricDatatypeDeclaration>(node->declarations[i]);
        if (pdecl) {
            // Build a sort representing the datatype (to be used in the signatures of the constructors and selectors)
            SortPtr typeSort =
                    make_shared<Sort>(make_shared<SimpleIdentifier>(node->sorts[i]->symbol));
            typeSort = ctx->getStack()->expand(typeSort);

            for (const auto& param : pdecl->parameters) {
                typeSort->arguments.push_back(make_shared<Sort>(make_shared<SimpleIdentifier>(param)));
            }

            std::vector<ConstructorDeclarationPtr>& constructors = pdecl->constructors;

            for (const auto& cons : constructors) {
                // Start building function info for current constructor
                string consName = cons->symbol->toString();
                std::vector<SortPtr> consSig;

                for (const auto& sel : cons->selectors) {
                    // Add sort of current selector to current constructor signature
                    consSig.push_back(ctx->getStack()->expand(sel->sort));

                    // Build function info for current selector
                    string selName = sel->symbol->toString();
                    std::vector<SortPtr> selSig;
                    selSig.push_back(typeSort);
                    selSig.push_back(ctx->getStack()->expand(sel->sort));

                    // Add selector function info
                    infos.push_back(make_shared<FunInfo>(selName, selSig, pdecl->parameters, node));
                }

                // Add constructor function info
                consSig.push_back(typeSort);
                infos.push_back(make_shared<FunInfo>(consName, consSig, pdecl->parameters, node));
            }
        } else {
            // Build a sort representing the datatype (to be used in the signatures of the constructors and selectors)
            SortPtr typeSort =
                    make_shared<Sort>(make_shared<SimpleIdentifier>(node->sorts[i]->symbol));
            typeSort = ctx->getStack()->expand(typeSort);

            SimpleDatatypeDeclarationPtr sdecl =
                    dynamic_pointer_cast<SimpleDatatypeDeclaration>(node->declarations[i]);

            for (const auto& cons : sdecl->constructors) {
                // Start building function info for current constructor
                string consName = cons->symbol->toString();
                std::vector<SortPtr> consSig;

                for (const auto& sel : cons->selectors) {
                    // Add sort of current selector to current constructor signature
                    consSig.push_back(ctx->getStack()->expand(sel->sort));

                    // Build function info for current selector
                    string selName = sel->symbol->toString();
                    std::vector<SortPtr> selSig;
                    selSig.push_back(typeSort);
                    selSig.push_back(ctx->getStack()->expand(sel->sort));

                    // Add selector function info
                    infos.push_back(make_shared<FunInfo>(selName, selSig, node));
                }

                // Add constructor function info
                consSig.push_back(typeSort);
                infos.push_back(make_shared<FunInfo>(consName, consSig, node));
            }
        }
    }

    return infos;
}

void SortednessChecker::loadTheory(const string& theory) {
    NodeErrorPtr err;
    loadTheory(theory, NodePtr(), err);
}

void SortednessChecker::loadTheory(const string& theory, const NodePtr& node, NodeErrorPtr& err) {
    string path = ctx->getConfiguration()->get(Configuration::Property::LOC_THEORIES) + theory
                  + ctx->getConfiguration()->get(Configuration::Property::FILE_EXT_THEORY);
    FILE* f = fopen(path.c_str(), "r");
    if (f) {
        fclose(f);

        ExecutionSettingsPtr settings = make_shared<ExecutionSettings>();
        settings->setInputFromFile(path);
        settings->setCoreTheoryEnabled(false);
        settings->setSortCheckContext(ctx);

        Execution exec(settings);
        if (exec.parse()) {
            if (exec.checkSortedness()) {
                //currentTheories[theory] = true;
            }
        } else {
            addError(ErrorMessages::buildTheoryUnloadable(theory), node, err);
        }
    } else {
        addError(ErrorMessages::buildTheoryUnknown(theory), node, err);
    }
}

void SortednessChecker::loadLogic(const string& logic, const NodePtr& node, NodeErrorPtr& err) {
    string path = ctx->getConfiguration()->get(Configuration::Property::LOC_LOGICS) + logic
                  + ctx->getConfiguration()->get(Configuration::Property::FILE_EXT_LOGIC);
    FILE* f = fopen(path.c_str(), "r");
    if (f) {
        fclose(f);

        ExecutionSettingsPtr settings = make_shared<ExecutionSettings>();
        settings->setInputFromFile(path);
        settings->setCoreTheoryEnabled(false);
        settings->setSortCheckContext(ctx);

        Execution exec(settings);
        if (exec.parse()) {
            exec.checkSortedness();
        } else {
            addError(ErrorMessages::buildLogicUnloadable(logic), node, err);
        }
    } else {
        addError(ErrorMessages::buildLogicUnknown(logic), node, err);
    }
}

SortednessChecker::NodeErrorPtr SortednessChecker::checkSort(const SortPtr& sort,
                                                             const NodePtr& source,
                                                             SortednessChecker::NodeErrorPtr& err) {
    string name = sort->identifier->toString();
    SortInfoPtr info = ctx->getStack()->getSortInfo(name);
    if (!info) {
        err = addError(ErrorMessages::buildSortUnknown(name, sort->rowLeft, sort->colLeft,
                                                       sort->rowRight, sort->colRight), source, err);

        for (const auto& arg : sort->arguments) {
            checkSort(arg, source, err);
        }
    } else {
        if (sort->arguments.size() != info->arity) {
            err = addError(ErrorMessages::buildSortArity(name, info->arity, sort->arguments.size(),
                                                         sort->rowLeft, sort->colLeft,
                                                         sort->rowRight, sort->colRight),
                           source, info, err);
        } else {
            for (const auto& arg : sort->arguments) {
                checkSort(arg, source, err);
            }
        }
    }
    return err;
}

SortednessChecker::NodeErrorPtr SortednessChecker::checkSort(const std::vector<SymbolPtr>& params,
                                                             const SortPtr& sort,
                                                             const NodePtr& source,
                                                             SortednessChecker::NodeErrorPtr& err) {
    string name = sort->identifier->toString();
    bool isParam = false;
    for (const auto& param : params) {
        if (name == param->toString())
            isParam = true;
    }

    if (!isParam) {
        SortInfoPtr info = ctx->getStack()->getSortInfo(name);
        if (!info) {
            err = addError(ErrorMessages::buildSortUnknown(name, sort->rowLeft, sort->colLeft,
                                                           sort->rowRight, sort->colRight), source, err);
            for (const auto& arg : sort->arguments) {
                checkSort(params, arg, source, err);
            }
        } else {
            if (sort->arguments.empty())
                return err;

            if (sort->arguments.size() != info->arity) {
                err = addError(ErrorMessages::buildSortArity(name, info->arity, sort->arguments.size(),
                                                             sort->rowLeft, sort->colLeft,
                                                             sort->rowRight, sort->colRight),
                               source, info, err);
            } else {
                for (const auto& arg : sort->arguments) {
                    checkSort(params, arg, source, err);
                }
            }
        }
    }

    return err;
}

void SortednessChecker::visit(const AssertCommandPtr& node) {
    TermSorter sorter(shared_from_this());
    SortPtr result = sorter.run(node->term);
    if (result) {
        string resstr = result->toString();
        if (resstr != SORT_BOOL) {
            TermPtr term = node->term;
            addError(ErrorMessages::buildAssertTermNotBool(term->toString(), resstr,
                                                           term->rowLeft, term->colLeft,
                                                           term->rowRight, term->colRight), node);
        }
    } else {
        TermPtr term = node->term;
        addError(ErrorMessages::buildAssertTermNotWellSorted(term->toString(),
                                                             term->rowLeft, term->colLeft,
                                                             term->rowRight, term->colRight), node);
    }
}

void SortednessChecker::visit(const DeclareConstCommandPtr& node) {
    NodeErrorPtr err;
    err = checkSort(node->sort, node, err);

    FunInfoPtr nodeInfo = getInfo(node);
    FunInfoPtr dupInfo = ctx->getStack()->tryAdd(nodeInfo);

    if (dupInfo) {
        addError(ErrorMessages::buildConstAlreadyExists(nodeInfo->name), node, dupInfo, err);
    }
}

void SortednessChecker::visit(const DeclareFunCommandPtr& node) {
    NodeErrorPtr err;
    for (const auto& param : node->parameters) {
        err = checkSort(param, node, err);
    }

    err = checkSort(node->sort, node, err);

    FunInfoPtr nodeInfo = getInfo(node);
    FunInfoPtr dupInfo = ctx->getStack()->tryAdd(nodeInfo);

    if (dupInfo) {
        addError(ErrorMessages::buildFunAlreadyExists(nodeInfo->name), node, dupInfo, err);
    }
}

void SortednessChecker::visit(const DeclareDatatypeCommandPtr& node) {
    NodeErrorPtr err;

    ParametricDatatypeDeclarationPtr pdecl =
            dynamic_pointer_cast<ParametricDatatypeDeclaration>(node->declaration);

    std::vector<SymbolInfoPtr> infos = getInfo(node);
    for (const auto& info : infos) {
        SortInfoPtr sortInfo = dynamic_pointer_cast<SortInfo>(info);
        if (sortInfo) {
            SortInfoPtr dupInfo = ctx->getStack()->tryAdd(sortInfo);

            if (dupInfo) {
                err = addError(ErrorMessages::buildSortAlreadyExists(sortInfo->name), node, dupInfo, err);
            }
        }
    }

    if (pdecl) {
        for (const auto& cons : pdecl->constructors) {
            for (const auto& sel : cons->selectors) {
                err = checkSort(pdecl->parameters, sel->sort, node, err);
            }
        }
    } else {
        SimpleDatatypeDeclarationPtr sdecl =
                dynamic_pointer_cast<SimpleDatatypeDeclaration>(node->declaration);

        for (const auto& cons : sdecl->constructors) {
            for (const auto& sel : cons->selectors) {
                err = checkSort(sel->sort, node, err);
            }
        }
    }

    infos = getInfo(node);
    for (const auto& info : infos) {
        FunInfoPtr funInfo = dynamic_pointer_cast<FunInfo>(info);
        if (funInfo) {
            FunInfoPtr dupInfo = ctx->getStack()->tryAdd(funInfo);

            if (dupInfo) {
                err = addError(ErrorMessages::buildFunAlreadyExists(funInfo->name), node, dupInfo, err);
            }

        }
    }
}

void SortednessChecker::visit(const DeclareDatatypesCommandPtr& node) {
    NodeErrorPtr err;

    std::vector<SymbolInfoPtr> infos = getInfo(node);
    for (const auto& info : infos) {
        SortInfoPtr sortInfo = dynamic_pointer_cast<SortInfo>(info);
        if (sortInfo) {
            SortInfoPtr dupInfo = ctx->getStack()->tryAdd(sortInfo);
            if (dupInfo) {
                err = addError(ErrorMessages::buildSortAlreadyExists(sortInfo->name), node, dupInfo, err);
            }
        }
    }

    for (size_t i = 0, sz = node->sorts.size(); i < sz; i++) {
        NodeErrorPtr declerr;

        ParametricDatatypeDeclarationPtr pdecl =
                dynamic_pointer_cast<ParametricDatatypeDeclaration>(node->declarations[i]);

        if (pdecl) {
            for (const auto& cons : pdecl->constructors) {
                for (const auto& sel : cons->selectors) {
                    declerr = checkSort(pdecl->parameters, sel->sort, pdecl, declerr);
                }
            }
        } else {
            SimpleDatatypeDeclarationPtr sdecl =
                    dynamic_pointer_cast<SimpleDatatypeDeclaration>(node->declarations[i]);

            for (const auto& cons : sdecl->constructors) {
                for (const auto& sel : cons->selectors) {
                    declerr = checkSort(sel->sort, sdecl, declerr);
                }
            }
        }
    }

    infos = getInfo(node);
    for (const auto& info : infos) {
        FunInfoPtr funInfo = dynamic_pointer_cast<FunInfo>(info);

        if (funInfo) {
            FunInfoPtr dupInfo = ctx->getStack()->tryAdd(funInfo);
            if (dupInfo) {
                err = addError(ErrorMessages::buildFunAlreadyExists(funInfo->name), node, dupInfo, err);
            }
        }
    }
}

void SortednessChecker::visit(const DeclareSortCommandPtr& node) {
    SortInfoPtr nodeInfo = getInfo(node);
    SortInfoPtr dupInfo = ctx->getStack()->tryAdd(nodeInfo);

    if (dupInfo) {
        addError(ErrorMessages::buildSortAlreadyExists(nodeInfo->name), node, dupInfo);
    }
}

void SortednessChecker::visit(const DeclareHeapCommandPtr& node) {
    NodeErrorPtr err;

    for(const auto& pair : node->locDataPairs) {
        err = checkSort(pair.first, node, err);
        err = checkSort(pair.second, node, err);
    }
}

void SortednessChecker::visit(const DefineFunCommandPtr& node) {
    NodeErrorPtr err;

    for (const auto& sort : node->definition->signature->parameters) {
        err = checkSort(sort->sort, node, err);
    }
    err = checkSort(node->definition->signature->sort, node, err);

    FunInfoPtr nodeInfo = getInfo(node);
    FunInfoPtr dupInfo = ctx->getStack()->findDuplicate(nodeInfo);

    if (dupInfo) {
        addError(ErrorMessages::buildFunAlreadyExists(nodeInfo->name), node, dupInfo, err);
    } else {
        ctx->getStack()->push();

        for (const auto& bind : node->definition->signature->parameters) {
            ctx->getStack()->tryAdd(make_shared<VarInfo>(bind->symbol->toString(),
                                                         ctx->getStack()->expand(bind->sort), node));
        }

        TermSorter sorter(shared_from_this());
        SortPtr result = sorter.run(node->definition->body);

        if (result) {
            string retstr = nodeInfo->signature[nodeInfo->signature.size() - 1]->toString();
            string resstr = result->toString();
            if (resstr != retstr) {
                TermPtr body = node->definition->body;
                addError(ErrorMessages::buildFunBodyWrongSort(body->toString(), resstr, retstr,
                                                              body->rowLeft, body->colLeft,
                                                              body->rowRight, body->colRight), node);
            }
        } else {
            TermPtr body = node->definition->body;
            addError(ErrorMessages::buildFunBodyNotWellSorted(body->toString(),
                                                              body->rowLeft, body->colLeft,
                                                              body->rowRight, body->colRight), node);
        }

        ctx->getStack()->pop();
        ctx->getStack()->tryAdd(nodeInfo);
    }
}

void SortednessChecker::visit(const DefineFunRecCommandPtr& node) {
    NodeErrorPtr err;

    for (const auto& sort : node->definition->signature->parameters) {
        err = checkSort(sort->sort, node, err);
    }
    err = checkSort(node->definition->signature->sort, node, err);

    FunInfoPtr nodeInfo = getInfo(node);
    FunInfoPtr dupInfo = ctx->getStack()->findDuplicate(nodeInfo);

    if (dupInfo) {
        addError(ErrorMessages::buildFunAlreadyExists(nodeInfo->name), node, dupInfo, err);
    } else {
        ctx->getStack()->push();
        ctx->getStack()->tryAdd(nodeInfo);

        for (const auto& bind : node->definition->signature->parameters) {
            ctx->getStack()->tryAdd(make_shared<VarInfo>(bind->symbol->toString(),
                                                         ctx->getStack()->expand(bind->sort), node));
        }

        TermSorter sorter(shared_from_this());
        SortPtr result = sorter.run(node->definition->body);

        if (result) {
            string retstr = nodeInfo->signature[nodeInfo->signature.size() - 1]->toString();
            string resstr = result->toString();
            if (resstr != retstr) {
                TermPtr body = node->definition->body;
                addError(ErrorMessages::buildFunBodyWrongSort(body->toString(), resstr, retstr,
                                                              body->rowLeft, body->colLeft,
                                                              body->rowRight, body->colRight), node);
            }
        } else {
            TermPtr body = node->definition->body;
            addError(ErrorMessages::buildFunBodyNotWellSorted(body->toString(),
                                                              body->rowLeft, body->colLeft,
                                                              body->rowRight, body->colRight), node);
        }

        ctx->getStack()->pop();
        ctx->getStack()->tryAdd(nodeInfo);
    }
}

void SortednessChecker::visit(const DefineFunsRecCommandPtr& node) {
    NodeErrorPtr err;

    for (const auto& decl : node->declarations) {
        std::vector<SortedVariablePtr>& sig = decl->parameters;
        for (const auto& itt : sig) {
            err = checkSort(itt->sort, node, err);
        }
        err = checkSort(decl->sort, node, err);
    }

    std::vector<FunInfoPtr> infos = getInfo(node);

    bool dup = false;
    for (const auto& info : infos) {
        FunInfoPtr dupInfo = ctx->getStack()->findDuplicate(info);
        if (dupInfo) {
            dup = true;
            err = addError(ErrorMessages::buildFunAlreadyExists(info->name), node, info, err);
        }
    }

    if (!dup) {
        ctx->getStack()->push();

        for (size_t i = 0, sz = node->declarations.size(); i < sz; i++) {
            ctx->getStack()->tryAdd(infos[i]);
        }

        for (size_t i = 0, sz = node->declarations.size(); i < sz; i++) {
            ctx->getStack()->push();
            std::vector<SortedVariablePtr>& bindings = node->declarations[i]->parameters;
            for (const auto& bind : bindings) {
                ctx->getStack()->tryAdd(make_shared<VarInfo>(bind->symbol->toString(),
                                                             ctx->getStack()->expand(bind->sort), node));
            }

            TermSorter sorter(shared_from_this());
            SortPtr result = sorter.run(node->bodies[i]);

            if (result) {
                string retstr = infos[i]->signature[infos[i]->signature.size() - 1]->toString();
                string resstr = result->toString();
                if (resstr != retstr) {
                    err = addError(ErrorMessages::buildFunBodyWrongSort(infos[i]->name, infos[i]->body->toString(),
                                                                        resstr, retstr, infos[i]->body->rowLeft,
                                                                        infos[i]->body->colLeft,
                                                                        infos[i]->body->rowRight,
                                                                        infos[i]->body->colRight), node, err);
                }
            } else {
                err = addError(ErrorMessages::buildFunBodyNotWellSorted(infos[i]->name, infos[i]->body->toString(),
                                                                        infos[i]->body->rowLeft,
                                                                        infos[i]->body->colLeft,
                                                                        infos[i]->body->rowRight,
                                                                        infos[i]->body->colRight), node, err);
            }
            ctx->getStack()->pop();
        }

        ctx->getStack()->pop();
        for (const auto& info : infos) {
            ctx->getStack()->tryAdd(info);
        }
    }
}

void SortednessChecker::visit(const DefineSortCommandPtr& node) {
    NodeErrorPtr err;
    err = checkSort(node->parameters, node->sort, node, err);

    SortInfoPtr nodeInfo = getInfo(node);
    SortInfoPtr dupInfo = ctx->getStack()->tryAdd(nodeInfo);

    if (dupInfo) {
        addError(ErrorMessages::buildSortAlreadyExists(nodeInfo->name), node, dupInfo, err);
    }
}

void SortednessChecker::visit(const GetValueCommandPtr& node) {
    NodeErrorPtr err;

    for (const auto& term : node->terms) {
        TermSorter sorter(shared_from_this());
        SortPtr result = sorter.run(term);
        if (!result) {
            err = addError(ErrorMessages::buildTermNotWellSorted(
                    term->toString(), term->rowLeft,
                    term->colLeft, term->rowRight,
                    term->colRight), node, err);
        }
    }
}

void SortednessChecker::visit(const PopCommandPtr& node) {
    size_t levels = (size_t) node->numeral->value;
    if (!ctx->getStack()->pop(levels)) {
        addError(ErrorMessages::buildStackUnpoppable(levels), node);
    }
}

void SortednessChecker::visit(const PushCommandPtr& node) {
    ctx->getStack()->push((size_t) node->numeral->value);
}

void SortednessChecker::visit(const ResetCommandPtr& node) {
    ctx->getStack()->reset();
    ctx->setCurrentLogic("");
    ctx->getCurrentTheories().clear();
}

void SortednessChecker::visit(const SetLogicCommandPtr& node) {
    NodeErrorPtr err;
    if (!ctx->getCurrentLogic().empty()) {
        addError(ErrorMessages::buildLogicAlreadySet(ctx->getCurrentLogic()), node);
    } else {
        string logic = node->logic->toString();
        ctx->setCurrentLogic(logic);
        loadLogic(logic, node, err);
    }
}

void SortednessChecker::visit(const LogicPtr& node) {
    for (const auto& attr : node->attributes) {
        if (attr->keyword->value == KW_THEORIES) {
            NodeErrorPtr err;

            CompAttributeValuePtr attrValue =
                    dynamic_pointer_cast<CompAttributeValue>(attr->value);

            for (const auto& compValue : attrValue->values) {
                string theory = compValue->toString();
                auto found = find(ctx->getCurrentTheories().begin(),
                                  ctx->getCurrentTheories().end(),
                                  theory);

                if (found != ctx->getCurrentTheories().end()) {
                    err = addError(ErrorMessages::buildTheoryAlreadyLoaded(theory), attr, err);
                } else {
                    ctx->getCurrentTheories().push_back(theory);
                    loadTheory(theory, attr, err);
                }
            }
        }
    }
}

void SortednessChecker::visit(const TheoryPtr& node) {
    for (const auto& attr : node->attributes) {
        if (attr->keyword->value == KW_SORTS || attr->keyword->value == KW_FUNS) {
            CompAttributeValuePtr val = dynamic_pointer_cast<CompAttributeValue>(attr->value);
            visit0(val->values);
        }
    }
}

void SortednessChecker::visit(const ScriptPtr& node) {
    visit0(node->commands);
}

void SortednessChecker::visit(const SortSymbolDeclarationPtr& node) {
    SortInfoPtr nodeInfo = getInfo(node);
    SortInfoPtr dupInfo = ctx->getStack()->tryAdd(nodeInfo);

    if (dupInfo) {
        addError(ErrorMessages::buildSortAlreadyExists(nodeInfo->name), node, dupInfo);
    }
}

void SortednessChecker::visit(const SpecConstFunDeclarationPtr& node) {
    NodeErrorPtr err;
    err = checkSort(node->sort, node, err);

    FunInfoPtr nodeInfo = getInfo(node);
    FunInfoPtr dupInfo = ctx->getStack()->tryAdd(nodeInfo);

    if (dupInfo) {
        addError(ErrorMessages::buildSpecConstAlreadyExists(nodeInfo->name), node, dupInfo, err);
    }
}

void SortednessChecker::visit(const MetaSpecConstFunDeclarationPtr& node) {
    NodeErrorPtr err;
    err = checkSort(node->sort, node, err);

    FunInfoPtr nodeInfo = getInfo(node);
    std::vector<FunInfoPtr> dupInfo = ctx->getStack()->getFunInfo(nodeInfo->name);

    if (!dupInfo.empty()) {
        err = addError(ErrorMessages::buildMetaSpecConstAlreadyExists(nodeInfo->name), node, dupInfo[0], err);
    } else {
        ctx->getStack()->tryAdd(nodeInfo);
    }
}

void SortednessChecker::visit(const SimpleFunDeclarationPtr& node) {
    NodeErrorPtr err;

    for (const auto& sort : node->signature) {
        err = checkSort(sort, node, err);
    }

    FunInfoPtr nodeInfo = getInfo(node);

    if (nodeInfo->assocL) {
        if (node->signature.size() != 3) {
            err = addError(ErrorMessages::buildLeftAssocParamCount(nodeInfo->name), node, err);
            nodeInfo->assocL = false;
        } else {
            SortPtr firstSort = node->signature[0];
            SortPtr returnSort = node->signature[2];

            if (firstSort->toString() != returnSort->toString()) {
                err = addError(ErrorMessages::buildLeftAssocRetSort(nodeInfo->name), node, err);
                nodeInfo->assocL = false;
            }
        }
    }

    if (nodeInfo->assocR) {
        if (node->signature.size() != 3) {
            err = addError(ErrorMessages::buildRightAssocParamCount(nodeInfo->name), node, err);
            nodeInfo->assocR = false;
        } else {
            SortPtr secondSort = node->signature[1];
            SortPtr returnSort = node->signature[2];

            if (secondSort->toString() != returnSort->toString()) {
                err = addError(ErrorMessages::buildRightAssocRetSort(nodeInfo->name), node, err);
                nodeInfo->assocR = false;
            }
        }
    }

    if (nodeInfo->chainable && nodeInfo->pairwise) {
        err = addError(ErrorMessages::buildChainableAndPairwise(nodeInfo->name), node, err);
        nodeInfo->chainable = false;
        nodeInfo->pairwise = false;
    } else if (nodeInfo->chainable) {
        if (node->signature.size() != 3) {
            err = addError(ErrorMessages::buildChainableParamCount(nodeInfo->name), node, err);
            nodeInfo->chainable = false;
        } else {
            SortPtr firstSort = node->signature[0];
            SortPtr secondSort = node->signature[1];
            SortPtr returnSort = node->signature[2];

            if (firstSort->toString() != secondSort->toString()) {
                err = addError(ErrorMessages::buildChainableParamSort(nodeInfo->name), node, err);
                nodeInfo->chainable = false;
            }

            if (returnSort->toString() != SORT_BOOL) {
                err = addError(ErrorMessages::buildChainableRetSort(nodeInfo->name), node, err);
                nodeInfo->chainable = false;
            }
        }
    } else if (nodeInfo->pairwise) {
        if (node->signature.size() != 3) {
            err = addError(ErrorMessages::buildPairwiseParamCount(nodeInfo->name), node, err);
            nodeInfo->pairwise = false;
        } else {
            SortPtr firstSort = node->signature[0];
            SortPtr secondSort = node->signature[1];
            SortPtr returnSort = node->signature[2];

            if (firstSort->toString() != secondSort->toString()) {
                err = addError(ErrorMessages::buildPairwiseParamSort(nodeInfo->name), node, err);
                nodeInfo->pairwise = false;
            }

            if (returnSort->toString() != SORT_BOOL) {
                err = addError(ErrorMessages::buildPairwiseRetSort(nodeInfo->name), node, err);
                nodeInfo->pairwise = false;
            }
        }
    }

    FunInfoPtr dupInfo = ctx->getStack()->tryAdd(nodeInfo);

    if (dupInfo) {
        addError(ErrorMessages::buildFunAlreadyExists(nodeInfo->name), node, dupInfo, err);
    }
}

void SortednessChecker::visit(const ParametricFunDeclarationPtr& node) {
    NodeErrorPtr err;

    for (const auto& sort : node->signature) {
        err = checkSort(node->parameters, sort, node, err);
    }

    FunInfoPtr nodeInfo = getInfo(node);

    if (nodeInfo->assocL) {
        if (node->signature.size() != 3) {
            err = addError(ErrorMessages::buildLeftAssocParamCount(nodeInfo->name), node, err);
            nodeInfo->assocL = false;
        } else {
            SortPtr firstSort = node->signature[0];
            SortPtr returnSort = node->signature[2];

            if (firstSort->toString() != returnSort->toString()) {
                err = addError(ErrorMessages::buildLeftAssocRetSort(nodeInfo->name), node, err);
                nodeInfo->assocL = false;
            }
        }
    }

    if (nodeInfo->assocR) {
        if (node->signature.size() != 3) {
            err = addError(ErrorMessages::buildRightAssocParamCount(nodeInfo->name), node, err);
            nodeInfo->assocR = false;
        } else {
            SortPtr secondSort = node->signature[1];
            SortPtr returnSort = node->signature[2];

            if (secondSort->toString() != returnSort->toString()) {
                err = addError(ErrorMessages::buildRightAssocRetSort(nodeInfo->name), node, err);
                nodeInfo->assocR = false;
            }
        }
    }

    if (nodeInfo->chainable && nodeInfo->pairwise) {
        err = addError(ErrorMessages::buildChainableAndPairwise(nodeInfo->name), node, err);
        nodeInfo->chainable = false;
        nodeInfo->pairwise = false;
    } else if (nodeInfo->chainable) {
        if (node->signature.size() != 3) {
            err = addError(ErrorMessages::buildChainableParamCount(nodeInfo->name), node, err);
            nodeInfo->chainable = false;
        } else {
            SortPtr firstSort = node->signature[0];
            SortPtr secondSort = node->signature[1];
            SortPtr returnSort = node->signature[2];

            if (firstSort->toString() != secondSort->toString()) {
                err = addError(ErrorMessages::buildChainableParamSort(nodeInfo->name), node, err);
                nodeInfo->chainable = false;
            }

            if (returnSort->toString() != SORT_BOOL) {
                err = addError(ErrorMessages::buildChainableRetSort(nodeInfo->name), node, err);
                nodeInfo->chainable = false;
            }
        }
    } else if (nodeInfo->pairwise) {
        if (node->signature.size() != 3) {
            err = addError(ErrorMessages::buildPairwiseParamCount(nodeInfo->name), node, err);
            nodeInfo->pairwise = false;
        } else {
            SortPtr firstSort = node->signature[0];
            SortPtr secondSort = node->signature[1];
            SortPtr returnSort = node->signature[2];

            if (firstSort->toString() != secondSort->toString()) {
                err = addError(ErrorMessages::buildPairwiseParamSort(nodeInfo->name), node, err);
                nodeInfo->pairwise = false;
            }

            if (returnSort->toString() != SORT_BOOL) {
                err = addError(ErrorMessages::buildPairwiseRetSort(nodeInfo->name), node, err);
                nodeInfo->pairwise = false;
            }
        }
    }

    FunInfoPtr dupInfo = ctx->getStack()->tryAdd(nodeInfo);

    if (dupInfo) {
        addError(ErrorMessages::buildFunAlreadyExists(nodeInfo->name), node, dupInfo, err);
    }
}

bool SortednessChecker::check(NodePtr& node) {
    if (node) {
        visit0(node);
    } else {
        Logger::warning("SortednessChecker::run()", "Attempting to check an empty abstract syntax tree");
        return false;
    }
    return errors.empty();
}

string SortednessChecker::getErrors() {
    stringstream ss;

    for (const auto& error : errors) {
        string file = error.first;

        if (!file.empty()) {
            size_t length = 11 + file.length();
            for (size_t i = 0; i < length; i++)
                ss << "-";
            ss << endl << "In file '" << file << "':" << endl;
            for (size_t i = 0; i < length; i++)
                ss << "-";
            ss << endl;
        }

        for (const auto& err : error.second) {
            if (err->node) {
                ss << err->node->rowLeft << ":" << err->node->colLeft
                   << " - " << err->node->rowRight << ":" << err->node->colRight << "   ";

                string nodestr = err->node->toString();
                if (nodestr.length() > 100)
                    ss << string(nodestr, 0, 100) << "[...]";
                else
                    ss << nodestr;

                ss << endl;
            }

            for (size_t i = 0, sz = err->errs.size(); i < sz; i++) {
                NodePtr source;

                if (err->errs[i]->info) {
                    source = err->errs[i]->info->source;
                }

                if (i != 0 && source)
                    ss << endl;

                ss << "\t" << err->errs[i]->message << "." << endl;

                if (source) {
                    ss << "\t\tPreviously, in file '" << source->filename->c_str() << "'\n\t\t"
                       << source->rowLeft << ":" << source->colLeft << " - "
                       << source->rowRight << ":" << source->colRight << "   ";

                    string sourcestr = source->toString();
                    if (sourcestr.length() > 100)
                        ss << string(sourcestr, 0, 100);
                    else
                        ss << sourcestr;

                    ss << endl;

                    if (i + 1 != sz)
                        ss << endl;
                }
            }

            ss << endl;
        }
    }

    ss << endl;

    return ss.str();
}

SymbolStackPtr SortednessChecker::getStack() {
    return ctx->getStack();
}

SortednessCheckerPtr SortednessChecker::getChecker() {
    return shared_from_this();
}

ConfigurationPtr SortednessChecker::getConfiguration() {
    return ctx->getConfiguration();
}
