#ifndef SLCOMP_PARSER_AST_SORTEDNESS_CHECKER_H
#define SLCOMP_PARSER_AST_SORTEDNESS_CHECKER_H

#include "ast_visitor_extra.h"
#include "ast_term_sorter.h"

#include "ast/ast_symbol_decl.h"
#include "ast/ast_command.h"
#include "stack/ast_symbol_stack.h"
#include "util/logger.h"
#include "util/configuration.h"

#include <map>

namespace smtlib {
    namespace ast {

        /* ============================= SortednessCheckerContext ============================= */
        class ISortCheckContext {
        public:
            virtual SymbolStackPtr getStack() = 0;
            virtual std::vector<std::string>& getCurrentTheories() = 0;
            virtual std::string getCurrentLogic() = 0;
            virtual ConfigurationPtr getConfiguration() = 0;
            virtual void setCurrentLogic(const std::string& logic) = 0;
        };

        typedef std::shared_ptr<ISortCheckContext> ISortCheckContextPtr;

        class SortednessCheckerContext : public ISortCheckContext,
                                         public std::enable_shared_from_this<SortednessCheckerContext> {
        private:
            SymbolStackPtr stack;
            std::vector<std::string> currentTheories;
            std::string currentLogic;
            ConfigurationPtr config;
        public:
            SortednessCheckerContext()
                    : stack(std::make_shared<SymbolStack>())
                    , config(std::make_shared<Configuration>()) {}

            inline explicit SortednessCheckerContext(SymbolStackPtr stack)
                    : stack(std::move(stack)) {}

            inline SymbolStackPtr getStack() override { return stack; }

            inline std::vector<std::string>& getCurrentTheories() override { return currentTheories; }

            inline std::string getCurrentLogic() override { return currentLogic; }

            inline ConfigurationPtr getConfiguration() override { return config; }

            inline void setCurrentLogic(const std::string& logic) override { this->currentLogic = logic; }
        };

        typedef std::shared_ptr<SortednessCheckerContext> SortCheckContextPtr;

        /* ================================ SortednessChecker ================================= */
        class SortednessChecker : public DummyVisitor0,
                                  public ITermSorterContext,
                                  public std::enable_shared_from_this<SortednessChecker> {
        public:
            struct Error {
                std::string message;
                SymbolInfoPtr info;

                inline explicit Error(std::string message)
                        : message(std::move(message)) {}

                inline Error(std::string message, SymbolInfoPtr info)
                        : message(std::move(message))
                        , info(std::move(info)) {}
            };

            typedef std::shared_ptr<Error> ErrorPtr;

            struct NodeError {
                std::vector<ErrorPtr> errs;
                NodePtr node;

                inline NodeError() = default;

                inline NodeError(ErrorPtr err, NodePtr node)
                        : node(std::move(node)) {
                    errs.push_back(std::move(err));
                }

                inline NodeError(std::vector<ErrorPtr> errs, NodePtr node)
                        : errs(std::move(errs))
                        , node(std::move(node)) {}
            };

            typedef std::shared_ptr<NodeError> NodeErrorPtr;

        private:
            ISortCheckContextPtr ctx;
            std::map<std::string, std::vector<NodeErrorPtr>> errors;

            SortInfoPtr getInfo(const SortSymbolDeclarationPtr& node);
            SortInfoPtr getInfo(const DeclareSortCommandPtr& node);
            SortInfoPtr getInfo(const DefineSortCommandPtr& node);

            FunInfoPtr getInfo(const SpecConstFunDeclarationPtr& node);
            FunInfoPtr getInfo(const MetaSpecConstFunDeclarationPtr& node);
            FunInfoPtr getInfo(const SimpleFunDeclarationPtr& node);
            FunInfoPtr getInfo(const ParametricFunDeclarationPtr& node);
            FunInfoPtr getInfo(const DeclareConstCommandPtr& node);
            FunInfoPtr getInfo(const DeclareFunCommandPtr& node);
            FunInfoPtr getInfo(const DefineFunCommandPtr& node);
            FunInfoPtr getInfo(const DefineFunRecCommandPtr& node);

            std::vector<FunInfoPtr> getInfo(const DefineFunsRecCommandPtr& node);
            std::vector<SymbolInfoPtr> getInfo(const DeclareDatatypeCommandPtr& node);
            std::vector<SymbolInfoPtr> getInfo(const DeclareDatatypesCommandPtr& node);

            void loadTheory(const std::string& theory,
                            const NodePtr& node,
                            NodeErrorPtr& err);

            void loadLogic(const std::string& logic,
                           const NodePtr& node,
                           NodeErrorPtr& err);

        public:
            inline SortednessChecker()
                    : ctx(std::make_shared<SortednessCheckerContext>()) {}

            inline explicit SortednessChecker(ISortCheckContextPtr ctx)
                    : ctx(std::move(ctx)) {}

            NodeErrorPtr addError(const std::string& message,
                                  const NodePtr& node,
                                  NodeErrorPtr& err);

            NodeErrorPtr addError(const std::string& message,
                                  const NodePtr& node,
                                  const SymbolInfoPtr& info,
                                  NodeErrorPtr& err);

            void addError(const std::string& message,
                          const NodePtr& node);

            void addError(const std::string& message,
                          const NodePtr& node,
                          const SymbolInfoPtr& err);

            void loadTheory(const std::string& theory);

            NodeErrorPtr checkSort(const SortPtr& sort,
                                   const NodePtr& source,
                                   NodeErrorPtr& err);

            NodeErrorPtr checkSort(const std::vector<SymbolPtr>& params,
                                   const SortPtr& sort,
                                   const NodePtr& source,
                                   NodeErrorPtr& err);

            void visit(const AssertCommandPtr& node) override;
            void visit(const DeclareConstCommandPtr& node) override;
            void visit(const DeclareFunCommandPtr& node) override;
            void visit(const DeclareDatatypeCommandPtr& node) override;
            void visit(const DeclareDatatypesCommandPtr& node) override;
            void visit(const DeclareSortCommandPtr& node) override;
            void visit(const DeclareHeapCommandPtr& node) override;
            void visit(const DefineFunCommandPtr& node) override;
            void visit(const DefineFunRecCommandPtr& node) override;
            void visit(const DefineFunsRecCommandPtr& node) override;
            void visit(const DefineSortCommandPtr& node) override;
            void visit(const GetValueCommandPtr& node) override;
            void visit(const PopCommandPtr& node) override;
            void visit(const PushCommandPtr& node) override;
            void visit(const ResetCommandPtr& node) override;
            void visit(const SetLogicCommandPtr& node) override;
            void visit(const LogicPtr& node) override;
            void visit(const TheoryPtr& node) override;
            void visit(const ScriptPtr& node) override;
            void visit(const SortSymbolDeclarationPtr& node) override;
            void visit(const SpecConstFunDeclarationPtr& node) override;
            void visit(const MetaSpecConstFunDeclarationPtr& node) override;
            void visit(const SimpleFunDeclarationPtr& node) override;
            void visit(const ParametricFunDeclarationPtr& node) override;

            bool check(NodePtr& node);

            std::string getErrors();

            // ITermSorterContext implementation
            SymbolStackPtr getStack() override;
            SortednessCheckerPtr getChecker() override;
            ConfigurationPtr getConfiguration() override;
        };

        typedef std::shared_ptr<SortednessChecker> SortednessCheckerPtr;
    }
}

#endif //SLCOMP_PARSER_AST_SORTEDNESS_CHECKER_H
