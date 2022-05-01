#pragma once

#include "ast.hpp"
#include "symbol.hpp"

#include <list>
#include <string>
#include <atomic>
#include <unordered_map>


class generator {
private:
    ast_class* _top_level = nullptr;
    std::list<std::string> _vm_code;
    std::unordered_map<std::string, symbol> _global_symbols;
    std::unordered_map<std::string, symbol> _subroutine_symbols;

    uint16_t _next_this_index = 0;
    uint16_t _next_local_index = 0;
    uint16_t _next_label = 0;
public:
    generator() = default;
    ~generator() = default;

    void run(ast_class* ast);

    [[nodiscard]] const std::list<std::string>& get_vm_code() const { return _vm_code; };

private:
    void _generate_subroutine(const ast_class_subroutine& subroutine);
    void _generate_statements(const std::list<ast_statement*>& statements);
    void _generate_let_statement(const ast_statement_let* let_statement);
    void _generate_if_statement(const ast_statement_if* if_statement);
    void _generate_while_statement(const ast_statement_while* while_statement);
    void _generate_do_statement(const ast_statement_do* do_statement);
    void _generate_return_statement(const ast_statement_return* return_statement);
    void _generate_expression(const ast_expression &expression);
    void _generate_term(const ast_term* term);
    void _generate_subroutine_call(ast_subroutine_call call);

    symbol _get_symbol(const std::string& identifier);
    std::optional<symbol> _try_get_symbol(const std::string& identifier);
private:
    static std::atomic<uint16_t> _next_static_index;

    static uint16_t _get_next_static_index();
public:
    static void reset();
};