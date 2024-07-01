#pragma once

#include <string>
#include <vector>

#include "utils/Context.h"
#include "ast/TranslationUnit.h"

namespace cpm {
    class TypeManager;
}

class Parser {
public:
    Parser(cpm::Context &context);

    ast::node_ptr<ast::TranslationUnit> parse() const;

    class SyntaxError : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

    class VisitError : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

private:
    cpm::Context &context;
};
