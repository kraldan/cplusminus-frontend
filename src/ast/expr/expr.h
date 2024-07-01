#pragma once

#include <variant>

// include classes that don't contain Expr
#include "IdExpr.h"
#include "ThisExpr.h"
#include "ast/expr/literals/IntLiteral.h"
#include "ast/expr/literals/CharLiteral.h"
#include "ast/expr/literals/BoolLiteral.h"
#include "ast/expr/literals/FloatLiteral.h"
#include "ast/expr/literals/NullptrLiteral.h"
#include "ast/expr/literals/StringLiteral.h"
#include "ast/expr/SizeofTypeExpr.h"
#include "ast/expr/ImplicitThisExpr.h"

namespace ast {
    // forward declare classes that contain Expr
    class BinaryExpr;

    class AssignmentExpr;

    class CommaExpr;

    class CallExpr;

    class SubscriptExpr;

    class TernaryExpr;

    class PostIncrExpr;

    class UnaryExpr;

    class CastExpr;

    class MemberAccessExpr;

    class DefaultArgExpr;

    class ImplicitTypeCastExpr;

    class LValToRValExpr;

    class ArrToPtrExpr;

    /**
     * Represents an expression.
     */
    using Expr = std::variant<
            IntLiteral,
            CharLiteral,
            BoolLiteral,
            FloatLiteral,
            StringLiteral,
            NullptrLiteral,
            IdExpr,
            ThisExpr,
            SizeofTypeExpr,
            ImplicitThisExpr,

            BinaryExpr,
            AssignmentExpr,
            CommaExpr,
            CallExpr,
            SubscriptExpr,
            TernaryExpr,
            PostIncrExpr,
            UnaryExpr,
            CastExpr,
            MemberAccessExpr,
            DefaultArgExpr,
            ImplicitTypeCastExpr,
            LValToRValExpr,
            ArrToPtrExpr
    >;
}

// include previously forward declared classes
#include "BinaryExpr.h"
#include "AssignmentExpr.h"
#include "CommaExpr.h"
#include "CallExpr.h"
#include "SubscriptExpr.h"
#include "TernaryExpr.h"
#include "PostIncrExpr.h"
#include "UnaryExpr.h"
#include "CastExpr.h"
#include "ast/class/MemberAccessExpr.h"
#include "DefaultArgExpr.h"
#include "ImplicitTypeCastExpr.h"
#include "LValToRValExpr.h"
#include "ArrToPtrExpr.h"


