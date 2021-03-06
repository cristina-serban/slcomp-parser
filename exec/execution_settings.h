/**
 * \file execution_settings.h
 * \brief Settings for execution handling.
 */

#ifndef SLCOMP_PARSER_EXECUTION_SETTINGS_H
#define SLCOMP_PARSER_EXECUTION_SETTINGS_H

#include "ast/ast_abstract.h"
#include "stack/ast_symbol_stack.h"
#include "util/global_typedef.h"
#include "visitor/ast_sortedness_checker.h"

#include <memory>

namespace slcompparser {
    class ExecutionSettings;
    typedef std::shared_ptr<ExecutionSettings> ExecutionSettingsPtr;

    /** Settings for execution handling. */
    class ExecutionSettings {
    public:
        enum InputMethod {
            INPUT_NONE = 0, INPUT_FILE, INPUT_AST
        };
    private:
        bool coreTheoryEnabled;
        std::string filename;

        smtlib::ast::NodePtr ast;
        smtlib::ast::ISortCheckContextPtr sortCheckContext;

        InputMethod inputMethod;
    public:
        /** Default constructor */
        ExecutionSettings();

        /** Copy constructor */
        explicit ExecutionSettings(const ExecutionSettingsPtr& settings);


        /** Whether the 'Core' theory is automatically loaded or not */
        inline bool isCoreTheoryEnabled() { return coreTheoryEnabled; }

        /** Set whether the 'Core' theory is automatically loaded or not */
        inline void setCoreTheoryEnabled(bool enabled) { coreTheoryEnabled = enabled; }


        /** Get the input method */
        inline InputMethod getInputMethod() { return inputMethod; }


        /** Get the input file */
        inline std::string getInputFile() { return filename; }

        /** Set a file as input */
        void setInputFromFile(std::string filename);


        /** Get the input AST */
        inline smtlib::ast::NodePtr getInputAst() { return ast; }

        /** Set an AST node as input */
        void setInputFromAst(smtlib::ast::NodePtr ast);


        /** Get the existing context for the sortedness check */
        inline smtlib::ast::ISortCheckContextPtr getSortCheckContext() {
            return sortCheckContext;
        }

        /** Set an existing context for the sortedness check */
        inline void setSortCheckContext(smtlib::ast::ISortCheckContextPtr ctx) {
            this->sortCheckContext = std::move(ctx);
        }
    };
}

#endif //SLCOMP_PARSER_EXECUTION_SETTINGS_H
