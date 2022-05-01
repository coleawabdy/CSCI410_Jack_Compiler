#include "generator.hpp"

#include <fmt/format.h>

#include <stdexcept>

#define GEN_DYNAMIC(fmt_str, ...) _vm_code.emplace_back(fmt::format(#fmt_str, __VA_ARGS__));
#define GEN(code) _vm_code.emplace_back(#code);

std::atomic_uint16_t generator::_next_static_index = 0;

void generator::run(ast_class *ast) {
    _next_this_index = 0;

    _top_level = ast;

    for(const auto& var : ast->variables) {
        for(const auto& identifier : var.identifiers) {
            if(_global_symbols.count(identifier) > 0)
                throw std::runtime_error("duplicate identifier '" + identifier + "'");

            if(var.is_static)
                _global_symbols[identifier] = symbol(symbol::segment_t::STATIC, _get_next_static_index());
            else
                _global_symbols[identifier] = symbol(symbol::segment_t::THIS, _next_this_index++);
        }
    }

    for(const auto& subroutine : ast->subroutines)
        _generate_subroutine(subroutine);
}

void generator::_generate_subroutine(const ast_class_subroutine &subroutine) {
    _subroutine_symbols.clear();
    _next_local_index = 0;

    for(const auto& local : subroutine.locals) {
        for(const auto& identifier : local.identifiers) {
            if(_global_symbols.count(identifier) > 0 || _subroutine_symbols.count(identifier))
                throw std::runtime_error("duplicate identifier '" + identifier + "'");

            _subroutine_symbols[identifier] = symbol(symbol::segment_t::LOCAL, _next_local_index++);
        }
    }

    GEN_DYNAMIC(function {}.{} {}, _top_level->identifier, subroutine.identifier, _subroutine_symbols.size())

    _generate_statements(subroutine.statements);
}

void generator::_generate_if_statement(const ast_statement_if *if_statement) {

}

void generator::_generate_let_statement(const ast_statement_let *let_statement) {
    if(let_statement->array_access.has_value()) {
        GEN_DYNAMIC(push {}, _get_symbol(let_statement->identifier).to_string())
        _generate_expression(let_statement->array_access.value());
        GEN(add)
        GEN(pop temp 0)
        _generate_expression(let_statement->assignment);
        GEN(push temp 0)
        GEN(pop pointer 1)
        GEN(pop that 0)
    } else {
        _generate_expression(let_statement->assignment);
        GEN_DYNAMIC(pop {}, _get_symbol(let_statement->identifier).to_string())
    }
}

void generator::_generate_while_statement(const ast_statement_while *while_statement) {
    auto label_num = _next_label++;
    auto begin_label = fmt::format("WHILE_BEGIN_{}", label_num);
    auto end_label = fmt::format("WHILE_END_{}", label_num);
    GEN_DYNAMIC(label {}, begin_label)
    _generate_expression(while_statement->conditional);
    GEN(not)
    GEN_DYNAMIC(if-goto {}, end_label)
    _generate_statements(while_statement->statements);
    GEN_DYNAMIC(goto {}, begin_label)
    GEN_DYNAMIC(label {}, end_label)
}

void generator::_generate_return_statement(const ast_statement_return *return_statement) {
    _generate_expression(return_statement->value);
    GEN(return)
}

void generator::_generate_do_statement(const ast_statement_do *do_statement) {
    _generate_subroutine_call(do_statement->call);
}

void generator::_generate_statements(const std::list<ast_statement *> &statements) {
    for(const auto& statement : statements) {
        switch(statement->type) {
            case ast_statement::type_t::IF:
                _generate_if_statement((ast_statement_if*)statement);
                break;
            case ast_statement::type_t::LET:
                _generate_let_statement((ast_statement_let*)statement);
                break;
            case ast_statement::type_t::WHILE:
                _generate_while_statement((ast_statement_while*)statement);
                break;
            case ast_statement::type_t::DO:
                _generate_do_statement((ast_statement_do*)statement);
                break;
            case ast_statement::type_t::RETURN:
                _generate_return_statement((ast_statement_return*)statement);
                break;
        }
    }
}

void generator::_generate_expression(const ast_expression &expression) {
    _generate_term(expression.primary);

    for(const auto& pair : expression.secondaries) {
        _generate_term(pair.second);
        switch(pair.first) {
            case ast_binary_op::ADD:
                GEN(add)
                break;
            case ast_binary_op::SUBTRACT:
                GEN(sub)
                break;
            case ast_binary_op::MULTIPLY:
                GEN(call Math.multiply 2)
                break;
            case ast_binary_op::DIVIDE:
                GEN(call Math.divide 2)
                break;
            case ast_binary_op::AND:
                GEN(and)
                break;
            case ast_binary_op::OR:
                GEN(or)
                break;
            case ast_binary_op::GREATER:
                GEN(gt)
                break;
            case ast_binary_op::LESSER:
                GEN(lt)
                break;
            case ast_binary_op::EQUAL:
                GEN(eq)
                break;
        }
    }
}

void generator::_generate_term(const ast_term *term) {
    switch(term->type) {
        case ast_term::type_t::INTEGER:
            GEN_DYNAMIC(push constant {}, ((ast_term_integer*)term)->value)
            break;
        case ast_term::type_t::STRING: {
            auto str_term = (ast_term_string*)term;
            GEN_DYNAMIC(push constant {}, str_term->value.length())
            GEN(call String.new 1)
            for(const auto& ch : str_term->value) {
                GEN_DYNAMIC(push constant {}, (uint16_t)ch)
                GEN(call String.appendChar 2)
            }
            break;
        }
        case ast_term::type_t::NUL:
            break;
        case ast_term::type_t::THIS:
            break;
        case ast_term::type_t::TRUE:
            break;
        case ast_term::type_t::FALSE:
            break;
        case ast_term::type_t::VARIABLE: {
            auto var_term = (ast_term_variable*)term;
            GEN_DYNAMIC(push {}, _get_symbol(var_term->identifier).to_string());
            break;
        }
        case ast_term::type_t::ARRAY: {
            auto array_term = (ast_term_array*)term;
            GEN_DYNAMIC(push {}, _get_symbol(array_term->identifier).to_string())
            _generate_expression(array_term->access);
            GEN(add)
            GEN(pop pointer 1)
            GEN(push that 0)
            break;
        }
        case ast_term::type_t::EXPRESSION:
            _generate_expression(((ast_term_expression*)term)->expression);
            break;
        case ast_term::type_t::UNARY:
            break;
        case ast_term::type_t::SUBROUTINE_CALL:
            _generate_subroutine_call(((ast_term_subroutine_call*)term)->call);
            break;
    }
}

void generator::_generate_subroutine_call(const ast_subroutine_call call) {

    for(const auto& param : call.arguments) {
        _generate_expression(param);
    }

    if(call.callee_identifier.has_value()) {
        auto symbol = _try_get_symbol(call.callee_identifier.value());
        if(symbol.has_value()) {

        } else {
            GEN_DYNAMIC(call {}.{} {}, call.callee_identifier.value(), call.subroutine_identifier, call.arguments.size())
        }
    }
}

std::optional<symbol> generator::_try_get_symbol(const std::string &identifier) {
    auto global_check = _global_symbols.find(identifier);
    if(global_check != _global_symbols.end())
        return global_check->second;

    auto subroutine_check = _subroutine_symbols.find(identifier);
    if(subroutine_check != _subroutine_symbols.end())
        return subroutine_check->second;

    return std::nullopt;
}


symbol generator::_get_symbol(const std::string& identifier) {
    auto get = _try_get_symbol(identifier);
    if(get.has_value())
        return get.value();

    throw std::runtime_error(fmt::format("symbol '{}' not found", identifier));
}

uint16_t generator::_get_next_static_index() {
    return _next_static_index.fetch_add(1, std::memory_order_relaxed);
}

void generator::reset() {
    _next_static_index = 0;
}