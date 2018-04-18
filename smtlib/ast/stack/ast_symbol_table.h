#ifndef SLCOMP_PARSER_AST_SYMBOL_TABLE_H
#define SLCOMP_PARSER_AST_SYMBOL_TABLE_H

#include "ast_symbol_util.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace smtlib {
    namespace ast {
        typedef std::unordered_map<std::string, SortInfoPtr> SortInfoMap;
        typedef std::unordered_map<std::string, std::vector<FunInfoPtr>> FunInfoMap;
        typedef std::unordered_map<std::string, VarInfoPtr> VarInfoMap;

        class SymbolTable {
        private:
            SortInfoMap sorts;
            FunInfoMap funs;
            VarInfoMap vars;

        public:
            inline SortInfoMap& getSorts() {
                return sorts;
            }

            inline FunInfoMap& getFuns() {
                return funs;
            }

            inline VarInfoMap& getVars() {
                return vars;
            }

            SortInfoPtr getSortInfo(const std::string& name);
            std::vector<FunInfoPtr> getFunInfo(const std::string& name);
            VarInfoPtr getVarInfo(const std::string& name);

            bool add(const SortInfoPtr& info);
            bool add(const FunInfoPtr& info);
            bool add(const VarInfoPtr& info);

            void reset();
        };

        typedef std::shared_ptr<SymbolTable> SymbolTablePtr;
    }
}

#endif //SLCOMP_PARSER_AST_SYMBOL_TABLE_H