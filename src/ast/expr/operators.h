#pragma once

#include <string>

namespace ast {
    enum BinaryOp {
        /* ARITHMETIC */
        Plus,
        Minus,
        Star,
        Div,
        Mod,
        /* BIT-WISE */
        And,
        Or,
        // xor
        Caret,
        LeftShift,
        RightShift,
        /* COMPARISON */
        Greater,
        Less,
        GreaterEqual,
        LessEqual,
        Equal,
        NotEqual,
        /* LOGICAL */
        LogicalAnd,
        LogicalOr,
    };

    std::string op_to_str(ast::BinaryOp op);

    bool is_arithmetic_op(ast::BinaryOp op);

    bool is_bit_op(ast::BinaryOp op);

    bool is_comp_op(ast::BinaryOp op);

    bool is_logical_op(ast::BinaryOp op);


    enum UnaryOp {
        UnStar,
        UnAnd,
        UnPlus,
        UnMinus,
        // ~
        BitNot,
        Not,
        PlusPlus,
        MinusMinus,
        Sizeof
    };

    std::string op_to_str(ast::UnaryOp op);

    enum AssignOp {
        Assign,
        PlusAssign,
        MinusAssign,
        StarAssign,
        DivAssign,
        ModAssign,
        AndAssign,
        OrAssign,
        CaretAssign,
        LeftShiftAssign,
        RightShiftAssign,
    };

    std::string op_to_str(ast::AssignOp op);

    /**
     * Converts assignment operator to binary operator.
     *
     * Throws "const char *" exception if the assignment op
     * does not have a binary op counterpart (e.g. assign '=').
     */
    BinaryOp assign_op_to_binary_op(ast::AssignOp op);
}
