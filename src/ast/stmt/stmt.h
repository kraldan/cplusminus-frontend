#pragma once

#include <variant>

// include classes that don't contain Stmt
#include "DeclarStmt.h"
#include "ExprStmt.h"
#include "BreakStmt.h"
#include "ContinueStmt.h"
#include "ReturnStmt.h"

namespace ast {
    // forward declare classes that contain Stmt
    class CompoundStmt;

    class DoWhileStmt;

    class IfStmt;

    class ForStmt;

    class WhileStmt;

    /**
     * Represents a statement.
     */
    using Stmt = std::variant<
            DeclarStmt,
            ExprStmt,
            BreakStmt,
            ContinueStmt,
            ReturnStmt,

            CompoundStmt,
            DoWhileStmt,
            ForStmt,
            IfStmt,
            WhileStmt
    >;
}

// include previously forward declared classes
#include "CompoundStmt.h"
#include "DoWhileStmt.h"
#include "ForStmt.h"
#include "IfStmt.h"
#include "WhileStmt.h"
