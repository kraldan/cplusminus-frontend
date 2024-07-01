#pragma once

/*
 * Includes all headers from ast/
 *
 * (right now doesn't actually include all files, but includes all the
 * classes and variants, since some headers are included by others)
 */

#include "ast/expr/expr.h"
#include "ast/decl/declaration.h"
#include "ast/decl/Decl.h"
#include "ast/decl/InitDeclarator.h"
#include "TranslationUnit.h"

#include "ast/decl/declaration.h"

#include "ast/func/Param.h"

#include "ast/stmt/stmt.h"
#include "ast/stmt/CompoundStmt.h"
#include "ast/stmt/DeclarStmt.h"
#include "ast/stmt/ExprStmt.h"

#include "ast/func/FuncBody.h"
#include "ast/func/FuncDef.h"

#include "ast/decl/SimpleDeclaration.h"
#include "ast/expr/CommaExpr.h"

#include "ast/stmt/BreakStmt.h"
#include "ast/stmt/ContinueStmt.h"
#include "ast/stmt/ReturnStmt.h"
#include "ast/stmt/DoWhileStmt.h"
#include "ast/stmt/forInitStmt.h"
#include "ast/stmt/ForStmt.h"
#include "ast/stmt/IfStmt.h"
#include "ast/stmt/WhileStmt.h"

#include "ast/stmt/Condition.h"

#include "ast/expr/CallExpr.h"
#include "ast/decl/EmptyDeclaration.h"
#include "ast/expr/SubscriptExpr.h"
#include "ast/expr/TernaryExpr.h"
#include "ast/expr/ThisExpr.h"

#include "ast/base/SourceInfo.h"
#include "ast/base/Node.h"

#include "ast/expr/PostIncrExpr.h"
#include "ast/expr/literals/BoolLiteral.h"
#include "ast/expr/literals/FloatLiteral.h"
#include "ast/expr/literals/NullptrLiteral.h"

#include "ast/expr/SizeofTypeExpr.h"
#include "ast/expr/UnaryExpr.h"
#include "ast/expr/CastExpr.h"

#include "ast/class/ClassHead.h"
#include "ast/class/ClassDef.h"
#include "ast/class/MemberDeclaration.h"
#include "ast/class/MemberDeclarator.h"
#include "ast/class/MemberDeclaratorList.h"
#include "ast/class/MemberSpecification.h"
#include "ast/class/MemberAccessExpr.h"

#include "ast/expr/DefaultArgExpr.h"
#include "ast/expr/ImplicitTypeCastExpr.h"
#include "ast/expr/LValToRValExpr.h"

#include "ast/decl/FunctionDecl.h"
#include "ast/decl/Decl.h"
