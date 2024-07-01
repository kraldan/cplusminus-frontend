#include <istream>
#include "Parser.h"
#include "parser/ParseTreeVisitor.h"
#include "CPMParser.h"
#include "CPMLexer.h"

Parser::Parser(cpm::Context &context) :
        context(context) {}

ast::node_ptr<ast::TranslationUnit> Parser::parse() const {
    // antlr parsing classes
    antlr4::ANTLRInputStream antlr_istream;
    CPMLexer antlr_lexer(&antlr_istream);
    antlr4::CommonTokenStream antlr_tokens(&antlr_lexer);
    CPMParser antlr_parser(&antlr_tokens);
    CPMParser::TranslationUnitContext *tu_ctx;
    // parse tree visitor
    ParserVisitor visitor{context};

    antlr_istream.load(context.getInput());
    tu_ctx = antlr_parser.translationUnit();
    if (antlr_parser.getNumberOfSyntaxErrors()) {
        throw Parser::SyntaxError("invalid syntax");
    }

    ast::node_ptr<ast::TranslationUnit> tu;
    try {
        tu = visitor.visitTranslationUnit(tu_ctx);
    }
    catch (const std::exception &e) {
        throw Parser::VisitError(e.what());
    }

    return tu;
}
