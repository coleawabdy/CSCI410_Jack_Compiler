#include "generator.hpp"

#include <fmt/format.h>

#include <stdexcept>

#define GEN_DYNAMIC(code) _vm_code.emplace_back(code);
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

    GEN_DYNAMIC(fmt::format("function {}.{} {}", _top_level->identifier, subroutine.identifier, subroutine.locals.size()))

    _generate_statements(subroutine.statements);
}

void generator::_generate_if_statement(const ast_statement_if *if_statement) {

}

void generator::_generate_let_statement(const ast_statement_let *let_statement) {

    _generate_expression(let_statement->assignment);

    if(let_statement->array_access.has_value()) {

    } else {
        GEN_DYNAMIC(fmt::format("pop {}", _get_symbol(let_statement->identifier).to_string()))
    }
}

void generator::_generate_while_statement(const ast_statement_while *while_statement) {

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
            GEN_DYNAMIC(fmt::format("push constant {}", ((ast_term_integer*)term)->value))
            break;
        case ast_term::type_t::STRING:
            break;
        case ast_term::type_t::NUL:
            break;
        case ast_term::type_t::THIS:
            break;
        case ast_term::type_t::TRUE:
            break;
        case ast_term::type_t::FALSE:
            break;
        case ast_term::type_t::VARIABLE:
            break;
        case ast_term::type_t::ARRAY:
            break;
        case ast_term::type_t::EXPRESSION:
            _generate_expression(((ast_term_expression*)term)->expression);
            break;
        case ast_term::type_t::UNARY:
            break;
        case ast_term::type_t::SUBROUTINE_CALL:
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
            GEN_DYNAMIC(fmt::format("call {}.{} {}", call.callee_identifier.value(), call.subroutine_identifier, call.arguments.size()))
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