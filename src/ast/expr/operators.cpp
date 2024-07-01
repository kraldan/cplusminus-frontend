#include "operators.h"

namespace ast {
    std::string op_to_str(BinaryOp op) {
        switch (op) {
            case BinaryOp::Plus:
                return "+";
            case BinaryOp::Minus:
                return "-";
            case BinaryOp::Star:
                return "*";
            case BinaryOp::Div:
                return "/";
            case BinaryOp::Mod:
                return "%";
            case BinaryOp::And:
                return "&";
            case BinaryOp::Or:
                return "|";
            case BinaryOp::Caret:
                return "^";
            case BinaryOp::LeftShift:
                return "<<";
            case BinaryOp::RightShift:
                return ">>";
            case BinaryOp::Greater:
                return ">";
            case BinaryOp::Less:
                return "<";
            case BinaryOp::GreaterEqual:
                return ">=";
            case BinaryOp::LessEqual:
                return "<=";
            case BinaryOp::Equal:
                return "==";
            case BinaryOp::NotEqual:
                return "!=";
            case BinaryOp::LogicalAnd:
                return "&&";
            case BinaryOp::LogicalOr:
                return "||";
            default:
                return "unknown operator";
        }
    }

    bool is_arithmetic_op(ast::BinaryOp op) {
        return op == BinaryOp::Plus
               || op == BinaryOp::Minus
               || op == BinaryOp::Star
               || op == BinaryOp::Div
               || op == BinaryOp::Mod;
    }

    bool is_bit_op(ast::BinaryOp op) {
        return op == BinaryOp::And
               || op == BinaryOp::Or
               || op == BinaryOp::Caret
               || op == BinaryOp::LeftShift
               || op == BinaryOp::RightShift;
    }

    bool is_comp_op(ast::BinaryOp op) {
        return op == BinaryOp::Equal
               || op == BinaryOp::NotEqual
               || op == BinaryOp::Greater
               || op == BinaryOp::Less
               || op == BinaryOp::GreaterEqual
               || op == BinaryOp::LessEqual;
    }

    bool is_logical_op(ast::BinaryOp op) {
        return op == BinaryOp::LogicalAnd
               || op == BinaryOp::LogicalOr;
    }


    std::string op_to_str(UnaryOp op) {
        switch (op) {
            case UnaryOp::UnStar:
                return "*";
            case UnaryOp::UnAnd:
                return "&";
            case UnaryOp::UnPlus:
                return "+";
            case UnaryOp::UnMinus:
                return "-";
            case UnaryOp::BitNot:
                return "~";
            case UnaryOp::Not:
                return "!";
            case UnaryOp::PlusPlus:
                return "++";
            case UnaryOp::MinusMinus:
                return "--";
            case UnaryOp::Sizeof:
                return "sizeof";
            default:
                return "unknown operator";
        }
    }


    std::string op_to_str(AssignOp op) {
        switch (op) {
            case AssignOp::Assign:
                return "=";
            case AssignOp::PlusAssign:
                return "+=";
            case AssignOp::MinusAssign:
                return "-=";
            case AssignOp::StarAssign:
                return "*=";
            case AssignOp::DivAssign:
                return "/=";
            case AssignOp::ModAssign:
                return "%=";
            case AssignOp::AndAssign:
                return "&=";
            case AssignOp::OrAssign:
                return "|=";
            case AssignOp::CaretAssign:
                return "^=";
            case AssignOp::LeftShiftAssign:
                return "<<=";
            case AssignOp::RightShiftAssign:
                return ">>=";
            default:
                return "unknown operator";
        }
    }

    ast::BinaryOp assign_op_to_binary_op(ast::AssignOp op) {
        switch (op) {
            case ast::AssignOp::PlusAssign:
                return ast::BinaryOp::Plus;
            case ast::AssignOp::MinusAssign:
                return ast::BinaryOp::Minus;
            case ast::AssignOp::StarAssign:
                return ast::BinaryOp::Star;
            case ast::AssignOp::DivAssign:
                return ast::BinaryOp::Div;
            case ast::AssignOp::ModAssign:
                return ast::BinaryOp::Mod;
            case ast::AssignOp::AndAssign:
                return ast::BinaryOp::And;
            case ast::AssignOp::OrAssign:
                return ast::BinaryOp::Or;
            case ast::AssignOp::CaretAssign:
                return ast::BinaryOp::Caret;
            case ast::AssignOp::LeftShiftAssign:
                return ast::BinaryOp::LeftShift;
            case ast::AssignOp::RightShiftAssign:
                return ast::BinaryOp::RightShift;
            default:
                throw "assign_op_to_binary_op invalid op";
        }
    }
}