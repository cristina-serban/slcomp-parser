#ifndef SLCOMP_PARSER_AST_SYMBOL_UTIL_H
#define SLCOMP_PARSER_AST_SYMBOL_UTIL_H

#include "ast/ast_basic.h"
#include "ast/ast_term.h"

#include <memory>
#include <string>

namespace smtlib {
    namespace ast {

        /* ==================================== SymbolInfo ==================================== */
        class SymbolInfo {
        public:
            std::string name;
            ast::NodePtr source;

            inline SymbolInfo() = default;

            inline SymbolInfo(std::string name, ast::NodePtr source)
                    : name(std::move(name))
                    , source(std::move(source)) {}

            virtual ~SymbolInfo();
        };

        typedef std::shared_ptr<SymbolInfo> SymbolInfoPtr;

        /* =================================== SortDefInfo ==================================== */
        class SortDefInfo {
        public:
            std::vector<ast::SymbolPtr> params;
            ast::SortPtr sort;

            inline SortDefInfo(std::vector<ast::SymbolPtr> params,
                               ast::SortPtr sort)
                    : params(std::move(params))
                    , sort(std::move(sort)) {}
        };

        typedef std::shared_ptr<SortDefInfo> SortDefInfoPtr;

        /* ===================================== SortInfo ===================================== */
        class SortInfo : public SymbolInfo {
        public:
            size_t arity;
            SortDefInfoPtr definition;
            std::vector<ast::AttributePtr> attributes;

            inline SortInfo(std::string name,
                            size_t arity,
                            ast::NodePtr source)
                    : SymbolInfo(std::move(name), std::move(source))
                    , arity(arity) {}

            inline SortInfo(std::string name,
                            size_t arity,
                            std::vector<ast::AttributePtr> attributes,
                            ast::NodePtr source)
                    : SymbolInfo(std::move(name), std::move(source))
                    , arity(arity)
                    , attributes(std::move(attributes)) {}

            inline SortInfo(std::string name,
                            size_t arity,
                            std::vector<ast::SymbolPtr> params,
                            ast::SortPtr sort,
                            ast::NodePtr source)
                    : SymbolInfo(std::move(name), std::move(source))
                    , arity(arity)
                    , definition(std::make_shared<SortDefInfo>(std::move(params), std::move(sort))) {}

            inline SortInfo(std::string name,
                            size_t arity,
                            std::vector<ast::SymbolPtr> params,
                            ast::SortPtr sort,
                            std::vector<ast::AttributePtr> attributes,
                            ast::NodePtr source)
                    : SymbolInfo(std::move(name), std::move(source))
                    , arity(arity)
                    , attributes(std::move(attributes))
                    , definition(std::make_shared<SortDefInfo>(std::move(params), std::move(sort))) {}
        };

        typedef std::shared_ptr<SortInfo> SortInfoPtr;

        /* ===================================== FunInfo ====================================== */
        class FunInfo : public SymbolInfo {
        public:
            std::vector<ast::SortPtr> signature;
            std::vector<ast::SymbolPtr> params;
            ast::TermPtr body;
            std::vector<ast::AttributePtr> attributes;

            bool assocR { false };
            bool assocL { false };
            bool chainable { false };
            bool pairwise { false };

            inline FunInfo(std::string name,
                           std::vector<ast::SortPtr> signature,
                           ast::NodePtr source)
                    : SymbolInfo(std::move(name), std::move(source))
                    , signature(std::move(signature)) {}

            inline FunInfo(std::string name,
                           std::vector<ast::SortPtr> signature,
                           ast::TermPtr body,
                           ast::NodePtr source)
                    : SymbolInfo(std::move(name), std::move(source))
                    , signature(std::move(signature))
                    , body(std::move(body)) {}

            inline FunInfo(std::string name,
                           std::vector<ast::SortPtr> signature,
                           std::vector<ast::SymbolPtr> params,
                           ast::NodePtr source)
                    : SymbolInfo(std::move(name), std::move(source))
                    , signature(std::move(signature))
                    , params(std::move(params)) {}

            inline FunInfo(std::string& name,
                           std::vector<ast::SortPtr> signature,
                           std::vector<ast::SymbolPtr> params,
                           ast::TermPtr body,
                           ast::NodePtr source)
                    : SymbolInfo(std::move(name), std::move(source))
                    , signature(std::move(signature))
                    , params(std::move(params))
                    , body(std::move(body)) {}

            inline FunInfo(std::string name,
                           std::vector<ast::SortPtr> signature,
                           std::vector<ast::AttributePtr> attributes,
                           ast::NodePtr source)
                    : SymbolInfo(std::move(name), std::move(source))
                    , signature(std::move(signature))
                    , attributes(std::move(attributes)) {}

            inline FunInfo(std::string name,
                           std::vector<ast::SortPtr> signature,
                           ast::TermPtr body,
                           std::vector<ast::AttributePtr> attributes,
                           ast::NodePtr source)
                    : SymbolInfo(std::move(name), std::move(source))
                    , signature(std::move(signature))
                    , body(std::move(body))
                    , attributes(std::move(attributes)) {}

            inline FunInfo(std::string name,
                           std::vector<ast::SortPtr> signature,
                           std::vector<ast::SymbolPtr> params,
                           std::vector<ast::AttributePtr> attributes,
                           ast::NodePtr source)
                    : SymbolInfo(std::move(name), std::move(source))
                    , signature(std::move(signature))
                    , params(std::move(params))
                    , attributes(std::move(attributes)) {}

            inline FunInfo(std::string name,
                           std::vector<ast::SortPtr> signature,
                           std::vector<ast::SymbolPtr> params,
                           ast::TermPtr body,
                           std::vector<ast::AttributePtr> attributes,
                           ast::NodePtr source)
                    : SymbolInfo(std::move(name), std::move(source))
                    , signature(std::move(signature))
                    , params(std::move(params))
                    , attributes(std::move(attributes))
                    , body(std::move(body)) {}
        };

        typedef std::shared_ptr<FunInfo> FunInfoPtr;

        /* ===================================== VarInfo ====================================== */
        class VarInfo : public SymbolInfo {
        public:
            ast::SortPtr sort;
            ast::TermPtr term;

            inline VarInfo(std::string name,
                           ast::SortPtr sort,
                           ast::NodePtr source)
                    : SymbolInfo(std::move(name), std::move(source))
                    , sort(std::move(sort)) {}

            inline VarInfo(std::string name,
                           ast::SortPtr sort,
                           ast::TermPtr term,
                           ast::NodePtr source)
                    : SymbolInfo(std::move(name), std::move(source))
                    , sort(std::move(sort))
                    , term(std::move(term)) {}
        };

        typedef std::shared_ptr<VarInfo> VarInfoPtr;
    }
}
#endif //SLCOMP_PARSER_AST_SYMBOL_UTIL_H