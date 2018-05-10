INC=-I./smtlib/ast -I./smtlib/sep -I./smtlib -I.

slcomp-parser:
	g++ -g -c -std=c++11 $(INC) exec/execution.cpp -o execution.o
	g++ -g -c -std=c++11 $(INC) exec/execution_settings.cpp -o execution_settings.o
	g++ -g -c -std=c++11 $(INC) smtlib/parser/smtlib-flex-lexer.l.c -o smtlib-flex-lexer.l.o
	g++ -g -c -std=c++11 $(INC) smtlib/parser/smtlib-bison-parser.y.c -o smtlib-bison-parser.y.o
	g++ -g -c -std=c++11 $(INC) smtlib/parser/smtlib-glue.cpp -o smtlib-glue.o
	g++ -g -c -std=c++11 $(INC) smtlib/parser/smtlib_parser.cpp -o smtlib_parser.o
	g++ -g -c -std=c++11 $(INC) smtlib/ast/ast_attribute.cpp -o ast_attribute.o
	g++ -g -c -std=c++11 $(INC) smtlib/ast/ast_basic.cpp -o ast_basic.o
	g++ -g -c -std=c++11 $(INC) smtlib/ast/ast_command.cpp -o ast_command.o
	g++ -g -c -std=c++11 $(INC) smtlib/ast/ast_datatype.cpp -o ast_datatype.o
	g++ -g -c -std=c++11 $(INC) smtlib/ast/ast_fun.cpp -o ast_fun.o
	g++ -g -c -std=c++11 $(INC) smtlib/ast/ast_identifier.cpp -o ast_identifier.o
	g++ -g -c -std=c++11 $(INC) smtlib/ast/ast_literal.cpp -o ast_literal.o
	g++ -g -c -std=c++11 $(INC) smtlib/ast/ast_logic.cpp -o ast_logic.o
	g++ -g -c -std=c++11 $(INC) smtlib/ast/ast_match.cpp -o ast_match.o
	g++ -g -c -std=c++11 $(INC) smtlib/ast/ast_script.cpp -o ast_script.o
	g++ -g -c -std=c++11 $(INC) smtlib/ast/ast_s_expr.cpp -o ast_sexp.o
	g++ -g -c -std=c++11 $(INC) smtlib/ast/ast_sort.cpp -o ast_sort.o
	g++ -g -c -std=c++11 $(INC) smtlib/ast/ast_symbol_decl.cpp -o ast_symbol_decl.o
	g++ -g -c -std=c++11 $(INC) smtlib/ast/ast_term.cpp -o ast_term.o
	g++ -g -c -std=c++11 $(INC) smtlib/ast/ast_theory.cpp -o ast_theory.o
	g++ -g -c -std=c++11 $(INC) smtlib/ast/ast_variable.cpp -o ast_var.o
	g++ -g -c -std=c++11 $(INC) smtlib/ast/stack/ast_symbol_stack.cpp -o ast_symbol_stack.o
	g++ -g -c -std=c++11 $(INC) smtlib/ast/stack/ast_symbol_table.cpp -o ast_symbol_table.o
	g++ -g -c -std=c++11 $(INC) smtlib/ast/stack/ast_symbol_util.cpp -o ast_symbol_util.o
	g++ -g -c -std=c++11 $(INC) smtlib/ast/visitor/ast_visitor.cpp -o ast_visitor.o
	g++ -g -c -std=c++11 $(INC) smtlib/ast/visitor/ast_syntax_checker.cpp -o ast_syntax_checker.o
	g++ -g -c -std=c++11 $(INC) smtlib/ast/visitor/ast_sortedness_checker.cpp -o ast_sortedness_checker.o
	g++ -g -c -std=c++11 $(INC) smtlib/ast/visitor/ast_term_sorter.cpp -o ast_term_sorter.o
	g++ -g -c -std=c++11 $(INC) smtlib/sep/sep_attribute.cpp -o sep_attribute.o
	g++ -g -c -std=c++11 $(INC) smtlib/sep/sep_basic.cpp -o sep_basic.o
	g++ -g -c -std=c++11 $(INC) smtlib/sep/sep_command.cpp -o sep_command.o
	g++ -g -c -std=c++11 $(INC) smtlib/sep/sep_datatype.cpp -o sep_datatype.o
	g++ -g -c -std=c++11 $(INC) smtlib/sep/sep_fun.cpp -o sep_fun.o
	g++ -g -c -std=c++11 $(INC) smtlib/sep/sep_identifier.cpp -o sep_identifier.o
	g++ -g -c -std=c++11 $(INC) smtlib/sep/sep_literal.cpp -o sep_literal.o
	g++ -g -c -std=c++11 $(INC) smtlib/sep/sep_logic.cpp -o sep_logic.o
	g++ -g -c -std=c++11 $(INC) smtlib/sep/sep_match.cpp -o sep_match.o
	g++ -g -c -std=c++11 $(INC) smtlib/sep/sep_script.cpp -o sep_script.o
	g++ -g -c -std=c++11 $(INC) smtlib/sep/sep_s_expr.cpp -o sep_sexp.o
	g++ -g -c -std=c++11 $(INC) smtlib/sep/sep_sort.cpp -o sep_sort.o
	g++ -g -c -std=c++11 $(INC) smtlib/sep/sep_symbol_decl.cpp -o sep_symbol_decl.o
	g++ -g -c -std=c++11 $(INC) smtlib/sep/sep_term.cpp -o sep_term.o
	g++ -g -c -std=c++11 $(INC) smtlib/sep/sep_theory.cpp -o sep_theory.o
	g++ -g -c -std=c++11 $(INC) smtlib/sep/sep_variable.cpp -o sep_var.o
	g++ -g -c -std=c++11 $(INC) smtlib/sep/stack/sep_symbol_stack.cpp -o sep_symbol_stack.o
	g++ -g -c -std=c++11 $(INC) smtlib/sep/stack/sep_symbol_table.cpp -o sep_symbol_table.o
	g++ -g -c -std=c++11 $(INC) smtlib/sep/stack/sep_symbol_util.cpp -o sep_symbol_util.o
	g++ -g -c -std=c++11 $(INC) smtlib/sep/visitor/sep_visitor.cpp -o sep_visitor.o
	g++ -g -c -std=c++11 $(INC) smtlib/sep/visitor/sep_visitor_stack.cpp -o sep_visitor_stack.o
	g++ -g -c -std=c++11 $(INC) smtlib/sep/visitor/sep_heap_checker.cpp -o sep_heap_checker.o
	g++ -g -c -std=c++11 $(INC) smtlib/sep/visitor/sep_term_sorter.cpp -o sep_term_sorter.o
	g++ -g -c -std=c++11 $(INC) smtlib/sep/transl/sep_translator.cpp -o sep_translator.o
	g++ -g -c -std=c++11 $(INC) util/configuration.cpp -o configuration.o
	g++ -g -c -std=c++11 $(INC) util/error_messages.cpp -o error_messages.o
	g++ -g -c -std=c++11 $(INC) util/global_values.cpp -o global_values.o
	g++ -g -c -std=c++11 $(INC) util/logger.cpp -o logger.o
	g++ -g -c -std=c++11 $(INC) main.cpp -o main.o
	g++ -g -o slcomp-parser smtlib-flex-lexer.l.o smtlib-bison-parser.y.o smtlib-glue.o \
	ast_attribute.o ast_basic.o ast_command.o ast_datatype.o ast_fun.o \
	ast_identifier.o ast_literal.o ast_logic.o ast_match.o ast_script.o \
	ast_sexp.o ast_sort.o ast_symbol_decl.o ast_term.o ast_theory.o \
	ast_var.o smtlib_parser.o ast_symbol_stack.o ast_symbol_table.o \
	ast_symbol_util.o error_messages.o logger.o global_values.o ast_visitor.o \
	ast_syntax_checker.o ast_sortedness_checker.o ast_term_sorter.o \
	sep_attribute.o sep_basic.o sep_command.o sep_datatype.o sep_fun.o sep_identifier.o \
	sep_literal.o sep_logic.o sep_match.o sep_script.o sep_sexp.o sep_sort.o \
	sep_symbol_decl.o sep_term.o sep_theory.o sep_var.o sep_symbol_stack.o \
	sep_symbol_table.o sep_symbol_util.o sep_visitor.o sep_visitor_stack.o \
	sep_heap_checker.o sep_term_sorter.o sep_translator.o \
	execution.o execution_settings.o configuration.o main.o -lfl
	rm -f *.o
clean:
	rm -f *.o
	rm -f slcomp-parser