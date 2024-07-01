#include "SemanticChecker.h"

using namespace std;
using namespace cpm::sc;

ast::node_ptr<ast::Expr> SemanticChecker::convert_error(
        SemanticChecker::Value from, cpm::Type *dest,
        const ast::Node &node, bool throw_error) {
    if (throw_error)
        error(from.str() + " is not convertible to type " +
              cpm::to_string(dest), node);
    else
        return nullptr;
}

ast::node_ptr<ast::Expr>
SemanticChecker::convert_to_rval(ast::node_ptr<ast::Expr> val, cpm::Type *dest,
                                 const ast::Node &node, bool throw_error) {
    Value from = process(val);
    return convert_to_rval(std::move(val), from, dest, node, throw_error);
}

bool SemanticChecker::is_numerical(cpm::SimpleType *simple_ty) {
    return is_integral(simple_ty) || is_floating(simple_ty);
}

SemanticChecker::Value SemanticChecker::operator()(ast::IntLiteral &) {
    return {getIntType(), RValue};
}

SemanticChecker::Value SemanticChecker::operator()(ast::AssignmentExpr &node) {
    Value lhs = process(node.lhs);
    Value rhs = process(node.rhs);
    if (lhs.valtype == RValue)
        error("cannot assign into " + lhs.str(), node);
    if (cpm::function_ty(lhs.type) || cpm::array_ty(lhs.type))
        error("cannot assign to object of type " + cpm::to_string(lhs.type), node);
    if (is_const(lhs.type))
        error("cannot assign to const-qualified object of type " + cpm::to_string(lhs.type), node);

    // simple assign
    if (node.op == ast::Assign) {
        node.rhs = convert_to_rval(std::move(node.rhs), rhs, lhs.type, node);
        node.lhs_type = lhs.type;
    }
    // compound assign
    else {
        ast::BinaryOp op = ast::assign_op_to_binary_op(node.op);
        const auto &[lhs_op_ty, rhs_op_ty, res_ty] = conversions_for_bin_op(lhs.type, rhs.type, op);
        if (lhs_op_ty == nullptr || rhs_op_ty == nullptr || res_ty == nullptr)
            error("invalid operands for binary op " + ast::op_to_str(node.op) + ": " +
                  to_string(lhs.type) + " and " +
                  to_string(rhs.type), node);
        node.rhs = convert_to_rval(std::move(node.rhs), rhs, rhs_op_ty, node);
        node.lhs_type = lhs_op_ty;
    }
    return {lhs.type, LValue};
}

SemanticChecker::Value SemanticChecker::operator()(ast::BinaryExpr &node) {
    ast::BinaryOp op = node.op;
    Value lhs_val = process(node.lhs);
    Value rhs_val = process(node.rhs);
    cpm::Type *lhs_ty = lhs_val.type;
    cpm::Type *rhs_ty = rhs_val.type;
    const auto &[lhs_dest_ty, rhs_dest_ty, res_ty] = conversions_for_bin_op(lhs_ty, rhs_ty, op);
    if (lhs_dest_ty == nullptr || rhs_dest_ty == nullptr || res_ty == nullptr)
        error("invalid operands for binary op " + ast::op_to_str(node.op) + ": " +
              to_string(lhs_ty) + " and " +
              to_string(rhs_ty), node);

    node.lhs = convert_to_rval(std::move(node.lhs), lhs_val, lhs_dest_ty, node);
    node.rhs = convert_to_rval(std::move(node.rhs), rhs_val, rhs_dest_ty, node);

    return {res_ty, RValue};
}

SemanticChecker::Value SemanticChecker::operator()(ast::CommaExpr &node) {
    assert(!node.expressions.empty());
    Value res{};
    for (auto &e: node.expressions)
        res = process(e);
    return res;
}

SemanticChecker::Value SemanticChecker::operator()(ast::CallExpr &node) {
    std::string func_name;
    vector<Value> args;
    // this is used exclusively for overload resolution, should not be used later;
    // the size() can be +1 of args in case of member access call
    vector<cpm::Type *> arg_types;
    // in what scope is the function
    Scope *scope = nullptr;
    //
    size_t offset;
    // global function or class constructor
    if (const auto *id_expr = get_if<ast::IdExpr>(node.called_func.get())) {
        func_name = id_expr->id;
        offset = 0;
        // constructor
        if (classes.contains(func_name)) {
            node.ctor_call = true;
            arg_types.push_back(getPointerType(getSimpleType(func_name, false), true));
            offset = 1;
            scope = classes.at(func_name).get();
        }
        // normal function call
        else {
            node.ctor_call = false;
            scope = global_scope;
        }
    }
    // method call
    else if (auto *member_access = get_if<ast::MemberAccessExpr>(node.called_func.get())) {
        Value object = process(member_access->object);
        cpm::SimpleType *object_type = get_underlying_type(object, member_access->ptr_access, node);
        // in case of '->', we need rvalue of the pointer
        if (member_access->ptr_access)
            member_access->object = convert_to_rval(std::move(member_access->object), object,
                                                    object.type, node);

        arg_types.push_back(getPointerType(object_type, true));
        scope = classes.at(object_type->getTypeId()).get();
        func_name = member_access->member;
        // the first argument to the low-level function is pointer to the class object, not CallExpr.args[0]
        offset = 1;
        if (func_name == object_type->getTypeId())
            error("invalid constructor call on an already existing object", node);
    } else
        error("invalid callee", node);

    for (auto &a: node.args) {
        args.push_back(process(a));
        arg_types.push_back(args.back().type);
    }

    ScopeValue called_func = resolveCallOverload(func_name, scope, arg_types, node);
    // check if we're calling private method/constructor outside of class
    if (auto *c = dynamic_cast<Class *>(scope)) {
        if (!inside_scope(c) && c->is_private(called_func.decl))
            error("access of private method/constructor", node);
    }
    // save the declarator pointer if this is an IdExpr
    if (auto *id_expr = get_if<ast::IdExpr>(node.called_func.get()))
        id_expr->var = called_func.decl;

    // save the declarator for this call
    const auto *func_decl = dynamic_cast<const ast::FunctionDecl *>(called_func.decl);
    assert(func_decl);
    node.func = func_decl;

    // convert arguments to param types
    cpm::FunctionType *func_type = cpm::function_ty(called_func.type);
    assert(func_type);
    vector<cpm::Type *> params = func_type->getParams();
    // in case of class method/constructor, params. should contain the implicit this parameter
    assert(params.size() >= offset);
    // converts args to param values
    for (size_t i = 0; i < node.args.size(); i++) {
        if (i < params.size() - offset)
            node.args[i] = convert_to_rval(std::move(node.args[i]), args[i], params[i + offset],
                                           node);
        // vararg arguments need to just be converted to rvalues
        //      note: this assumes potential array arguments have undergone array-to-pointer decay
        else
            node.args[i] = convert_to_rval(std::move(node.args[i]), args[i], args[i].type, node);
    }

    // add default args if needed; they're already converted to rvalues of correct types
    for (size_t i = node.args.size() + offset; i < params.size(); i++) {
        ast::Expr *def_arg_expr = funcs_def_args.at(func_decl).at(i);
        assert(def_arg_expr);
        ast::node_ptr<ast::Expr> def_arg =
                ast::make_node<ast::DefaultArgExpr, ast::Expr>(node.src_info, def_arg_expr);
        node.args.push_back(std::move(def_arg));
    }
    cpm::Type *res_type = node.ctor_call ? getSimpleType(func_name, false)
                                         : func_type->getRetType();
    return {res_type, RValue};
}

SemanticChecker::Value SemanticChecker::operator()(ast::SubscriptExpr &node) {
    Value lhs = process(node.dest),
            index = process(node.index);
    cpm::Type *res_ty;
    cpm::PointerType *lhs_ptr_ty = cpm::pointer_ty(lhs.type);

    // handle dest, array has been decayed to pointer
    if (lhs_ptr_ty) {
        res_ty = lhs_ptr_ty->getElemType();
        if (incomplete_type(res_ty))
            error("cannot index object that points to incomplete type: " +
                  to_string(res_ty), node);
    } else
        error("type cannot be indexed: " + cpm::to_string(lhs.type), node);

    // this does potentially need ArrayToPointerDecay or LValToRVal conversion
    node.dest = convert_to_rval(std::move(node.dest), lhs, getPointerType(res_ty, false), node);

    // handle index
    if (!is_integral(index.type))
        error("index type is not integral: " + cpm::to_string(index.type), node);
    node.index = convert_to_rval(std::move(node.index), index, getIntType(), node);

    return {res_ty, LValue};
}

SemanticChecker::Value SemanticChecker::operator()(ast::TernaryExpr &node) {
    // condition has to be bool
    node.cond = convert_to_rval(std::move(node.cond), getBoolType(), node);
    Value then = process(node.then);
    Value else_ = process(node.else_);
    cpm::Type *common_type = determine_common_type(then.type, else_.type);
    if (!common_type)
        error(cpm::to_string(then.type) + " and " + cpm::to_string(else_.type) +
              " cannot be converted into a common type", node);

    // if both are lvalues and of the same type, we return lvalue
    // note: I'm not sure that they have to be exactly of the same type, this
    //      might skip over some viable cases
    if (then.type == else_.type)
        if (then.valtype == LValue && else_.valtype == LValue)
            return {then.type, LValue};

    node.then = convert_to_rval(std::move(node.then), then, common_type, node);
    node.else_ = convert_to_rval(std::move(node.else_), else_, common_type, node);

    return {common_type, RValue};
}

cpm::SimpleType *SemanticChecker::getIntType(bool is_const) {
    return getSimpleType("int", is_const);
}

cpm::SimpleType *SemanticChecker::getBoolType(bool is_const) {
    return getSimpleType("bool", is_const);
}

cpm::SimpleType *SemanticChecker::getVoidType() {
    return getSimpleType("void", false);
}

SemanticChecker::Value SemanticChecker::operator()(ast::CharLiteral &) {
    return {getCharType(), RValue};
}

SemanticChecker::Value SemanticChecker::operator()(ast::BoolLiteral &) {
    return {getBoolType(), RValue};
}

SemanticChecker::Value SemanticChecker::operator()(ast::FloatLiteral &) {
    return {getDoubleType(), RValue};
}

SemanticChecker::Value SemanticChecker::operator()(ast::NullptrLiteral &) {
    return {getNullptrType(), RValue};
}

SemanticChecker::Value SemanticChecker::operator()(ast::StringLiteral &node) {
    // +1 in size for the terminating \0
    return {getArrayType(getCharType(true), node.str.length() + 1), LValue};
}

SemanticChecker::Value SemanticChecker::operator()(ast::PostIncrExpr &node) {
    Value expr = process(node.expr);
    string op_str = node.incr ? "++"s : "--"s;
    if (expr.valtype != LValue)
        error("operator " + op_str + " requires lvalue", node);
    if (!viable_incr_type(expr.type))
        error("invalid type for operator " + op_str + ": " + cpm::to_string(expr.type), node);
    return {expr.type, RValue};
}

bool SemanticChecker::viable_incr_type(cpm::Type *t) {
    if (pointer_to_complete_type(t))
        return true;
    // bool is not viable to increment not according to C++17
    if (cpm::SimpleType *st = cpm::simple_ty(t))
        if (is_integral(st) && st->getTypeId() != "bool")
            return true;
    return false;
}

SemanticChecker::Value SemanticChecker::operator()(ast::UnaryExpr &node) {
    Value expr = process(node.expr, node.op != ast::UnAnd && node.op != ast::Sizeof);
    switch (node.op) {
        case ast::UnStar: {
            cpm::PointerType *ptr_ty = pointer_to_complete_type(expr.type);
            if (!ptr_ty)
                break;
            node.expr = convert_to_rval(std::move(node.expr), expr, ptr_ty, node);
            return {ptr_ty->getElemType(), LValue};
        }
        case ast::UnAnd:
            if (expr.valtype != LValue)
                error("cannot take address of " + expr.str(), node);
            return {getPointerType(expr.type, false), RValue};
        case ast::UnPlus:
            node.expr = convert_to_rval(std::move(node.expr), expr, expr.type, node);
            if (is_numerical(expr.type) || cpm::pointer_ty(expr.type))
                return {expr.type, RValue};
            break;
        case ast::UnMinus:
            node.expr = convert_to_rval(std::move(node.expr), expr, expr.type, node);
            if (is_numerical(expr.type))
                return {expr.type, RValue};
            break;
        case ast::BitNot:
            node.expr = convert_to_rval(std::move(node.expr), expr, expr.type, node);
            if (is_integral(expr.type))
                return {expr.type, RValue};
            break;
        case ast::Not:
            node.expr = convert_to_rval(std::move(node.expr), expr, getBoolType(), node);
            return {getBoolType(), RValue};
        case ast::PlusPlus:
        case ast::MinusMinus:
            if (!viable_incr_type(expr.type) || expr.valtype != LValue)
                break;
            return {expr.type, LValue};
        case ast::Sizeof:
            compiler_error("sizeof(expr) is expected to be handled out of UnaryExpr", node);
    }
    error("invalid " + expr.str() + " to unary operator '" + ast::op_to_str(node.op), node);
}

bool SemanticChecker::is_numerical(cpm::Type *type) {
    return is_numerical(cpm::simple_ty(type));
}

bool SemanticChecker::is_integral(cpm::Type *type) {
    return is_integral(cpm::simple_ty(type));
}

bool SemanticChecker::is_integral(cpm::SimpleType *simple_ty) {
    if (!simple_ty)
        return false;
    const string &id = simple_ty->getTypeId();
    return id == "int" || id == "char" || id == "bool";
}

void SemanticChecker::operator()(ast::BreakStmt &node) {
    if (node.break_level == 0)
        error("0 is invalid break level", node);
    else if (node.break_level > loop_level)
        error("not enough nested loops to break, current loop level: " + std::to_string(loop_level),
              node);
}

void SemanticChecker::operator()(ast::ContinueStmt &node) {
    if (node.continue_level == 0)
        error("0 is invalid continue level", node);
    else if (node.continue_level > loop_level)
        error("not enough nested loops to continue, current loop level: " +
              std::to_string(loop_level), node);
}

void SemanticChecker::operator()(ast::ReturnStmt &node) {
    assert(curr_ret_type && "return statement outside of a function");

    if (node.expr)
        node.expr = convert_to_rval(std::move(node.expr.value()), curr_ret_type, node);
    else if (!is_void(curr_ret_type))
        error("implicit void return statement in non-void function", node);
}

bool SemanticChecker::is_void(cpm::Type *type) {
    return is_void(cpm::simple_ty(type));
}

bool SemanticChecker::is_void(cpm::SimpleType *simple_ty) {
    return simple_ty && simple_ty->getTypeId() == "void";
}

void SemanticChecker::operator()(ast::CompoundStmt &node) {
    bool added_scope = cs_new_scope;
    if (added_scope)
        addScope();
    else
        // nested compound statements
        cs_new_scope = true;

    for (auto &s: node.statements)
        process(*s);

    if (added_scope)
        dropScope();
}

void SemanticChecker::operator()(ast::DoWhileStmt &node) {
    process(*node.cond);
    loop_level += 1;
    process(*node.body);
    loop_level -= 1;
}

void SemanticChecker::operator()(ast::ForStmt &node) {
    addScope();
    process(*node.initStmt);
    if (node.cond)
        process(*node.cond.value());
    if (node.post_iter)
        process(node.post_iter.value());
    cs_new_scope = false;
    loop_level += 1;
    process(*node.body);
    loop_level -= 1;
    dropScope();
    // reset, esp for case when body was not CompoundStmt
    cs_new_scope = true;
}

void SemanticChecker::operator()(ast::IfStmt &node) {
    process(*node.cond);
    process(*node.body);
    if (node.else_body)
        process(*node.else_body.value());
}

void SemanticChecker::operator()(ast::WhileStmt &node) {
    process(*node.cond);
    loop_level += 1;
    process(*node.body);
    loop_level -= 1;
}

void SemanticChecker::operator()(ast::ExprStmt &node) {
    if (node.expr)
        process(node.expr.value());
}

void SemanticChecker::operator()(ast::Condition &node) {
    node.expr = convert_to_rval(std::move(node.expr), getBoolType(), node);
}

SemanticChecker::Value SemanticChecker::operator()(ast::IdExpr &node) {
    vector<ScopeValue> vals = current_scope->getValues(node.id, true);
    if (vals.empty())
        error("unknown identifier: '" + node.id + "'", node);
    else if (vals.size() > 1)
        error("multiple values under id: '" + node.id + "'", node);
    if (cpm::function_ty(vals[0].decl->type))
        error("function used in a different context than function call", node);
    node.var = vals[0].decl;
    return {vals[0].type, LValue};
}

bool SemanticChecker::type_exists(const string &type_id) {
    return type_id == "double" ||
           type_id == "int" ||
           type_id == "char" ||
           type_id == "bool" ||
           type_id == "void" ||
           classes.contains(type_id);
}

void SemanticChecker::operator()(ast::DeclarStmt &node) {
    process(*node.declaration);
}

void SemanticChecker::operator()(ast::InitDeclarator &node) {
    // check if the type is viable
    process(*node.declarator);

    ast::Decl *decl = node.declarator.get();

    // function declarations are handled separately
    if (cpm::function_ty(decl->type)) {
        if (current_scope != global_scope)
            // class doesn't contain InitDeclarators, so checking for global scope is enough
            error("function declaration outside the global scope or class scope", node);
        auto *func_decl = dynamic_cast<ast::FunctionDecl *>(decl);
        assert(func_decl);
        declareFunction(func_decl);
        if (node.initializer)
            error("functions cannot be initialized", node);
        return;
    }
    if (is_void(decl->type))
        error("cannot declare variable of type 'void'", node);

    if (current_scope->contains(decl->id))
        error("redeclaration of name '" + decl->id + "'", node);

    current_scope->addValue(decl->id, decl->type, node.declarator.get());

    if (node.initializer) {
        // note: c++ would not decay string literal array to pointer here, but we do
        //       because we don't support direct array initialization by literal
        Value init = process(node.initializer.value());
        node.initializer = convert_to_rval(std::move(node.initializer.value()), init, decl->type,
                                           node, false);
        if (node.initializer == nullptr)
            error("cannot initialize value of type " + cpm::to_string(decl->type) + " with " +
                  init.str(), node);
    }
}

void SemanticChecker::operator()(ast::EmptyDeclaration &) {
    // do nothing
}

void SemanticChecker::operator()(ast::SimpleDeclar &node) {
    for (auto &id: node.init_declars)
        process(*id);
}

void SemanticChecker::operator()(ast::FuncBody &node) {
    process(*node.comp_stmt);
}

void SemanticChecker::operator()(ast::TranslationUnit &node) {
    global_scope = addScope();
    add_builtin_functions(node);
    for (const auto &d: node.declars)
        process(*d);
}

bool SemanticChecker::is_const(cpm::Type *type) {
    if (cpm::SimpleType *st = cpm::simple_ty(type))
        return st->isConst();
    else if (cpm::PointerType *pt = cpm::pointer_ty(type))
        return pt->isConst();
    return false;
}

const ast::FunctionDecl *
SemanticChecker::declareFunction(ast::FunctionDecl *func_decl, bool process_def_args) {
    // check if the function declarator is valid
    process(*func_decl, process_def_args);
    cpm::FunctionType *func_type = cpm::function_ty(func_decl->type);
    assert(func_type);

    if (func_decl->id == "main") {
        if (!is_int(func_type->getRetType()))
            error("'main' must return 'int'", *func_decl);
    }

    const ast::FunctionDecl *original_decl = addFunctionToScope(func_decl);
    if (original_decl != func_decl)
        func_decl->orig = original_decl;

    // set default argument values
    if (process_def_args) {
        vector<ast::Expr *> &default_args = funcs_def_args[original_decl];
        bool def_args_started = false;
        for (size_t i = 0; i < default_args.size(); i++) {
            ast::node_ptr<ast::Param> &p = func_decl->params[i];
            if (!p->default_val.has_value()) {
                if (def_args_started)
                    error("gap in default arguments", *func_decl);
                continue;
            }
            def_args_started = true;
            if (default_args[i] != nullptr)
                error("redefinition of default arguments", *func_decl);
            // make any implicit conversions needed on the default val
            p->default_val = convert_to_rval(std::move(p->default_val.value()),
                                             func_type->getParams()[i],
                                             *func_decl);
            default_args[i] = p->default_val.value().get();
            def_arg_vals[p->default_val.value().get()] = {func_type->getParams()[i], RValue};
        }
    }

    return original_decl;
}

const ast::FunctionDecl *SemanticChecker::addFunctionToScope(const ast::FunctionDecl *func_decl) {
    const string &id = func_decl->id;
    cpm::FunctionType *func_type = cpm::function_ty(func_decl->type);
    assert(func_type);
    vector<ScopeValue> functions = current_scope->getValues(id, false);
    if (!functions.empty()) {
        if (!cpm::function_ty(functions[0].type))
            error("'" + id + "' already declared as a different kind of symbol", *func_decl);
        // compare each known overload
        for (const auto &[decl, type]: functions) {
            cpm::FunctionType *cmp_func = cpm::function_ty(type);
            assert(cmp_func);
            switch (cmpFuncSignatures(func_type, cmp_func)) {
                case MATCH: {
                    const auto *first_func_decl = dynamic_cast<const ast::FunctionDecl *>(decl);
                    assert(first_func_decl);
                    return first_func_decl;
                }
                case RET_DIFF:
                    error("function differs from a previous declaration only in return type",
                          *func_decl);
                case NO_MATCH:
                    break;
            }
        }
    }
    // class methods differentiate between public and private
    if (defined_class.empty())
        current_scope->addValue(id, func_type, func_decl);
    else
        classes[defined_class]->addValue(id, func_type, func_decl, current_access);
    funcs_def_args[func_decl] = vector<ast::Expr *>(func_type->getParams().size(), nullptr);
    return func_decl;
}

void SemanticChecker::operator()(ast::FuncDef &node) {
    auto *func_decl = node.declarator.get();
    cpm::FunctionType *func_type = cpm::function_ty(func_decl->type);
    assert(func_type);
    // constructor
    if (node.ctor) {
        // check that the parser set the return type to void
        assert(func_type->getRetType() == getVoidType());
        if (defined_class != node.declarator->id)
            error("invalid constructor definition", node);
    }
    const ast::Decl *orig_decl = declareFunction(func_decl);

    // handle redefinition
    if (defined_funcs.contains(orig_decl))
        error("redefinition of function '" + func_decl->id + "'", node);
    else
        defined_funcs.insert(orig_decl);

    addScope();
    const vector<ast::node_ptr<ast::Param>> &params = func_decl->params;
    // note: 'this' is not added to the scope since it doesn't have an ast::Decl
    //      see operator() (ThisExpr)
    for (const auto &param: params) {
        // note: it is important to use 'param_decl.type' and not 'func_type->getParams()[i]'
        // or something, because we need the (lack of) const qualifiers from the most recent
        // param_decl
        current_scope->addValue(param->declarator->id, param->declarator->type,
                                param->declarator.get());
    }
    cs_new_scope = false;
    curr_ret_type = func_type->getRetType();
    process(*node.body);
    curr_ret_type = nullptr;
    dropScope();
}

SemanticChecker::FuncCMP
SemanticChecker::cmpFuncSignatures(cpm::FunctionType *t1, cpm::FunctionType *t2) {
    if (t1->getParams().size() != t2->getParams().size() || t1->isVararg() != t2->isVararg())
        return NO_MATCH;
    for (size_t i = 0; i < t1->getParams().size(); i++)
        if (const_unqualified_type(t1->getParams()[i]) !=
            const_unqualified_type(t2->getParams()[i]))
            return NO_MATCH;
    if (t1->getRetType() != t2->getRetType())
        return RET_DIFF;
    return MATCH;
}

ScopeValue
SemanticChecker::resolveCallOverload(const std::string &func_name, Scope *scope,
                                     const vector<cpm::Type *> &arg_types,
                                     const ast::CallExpr &node) {
    const size_t arg_count = arg_types.size();
    vector<ScopeValue> candidates = scope->getValues(func_name, true);
    // the vector<TypeMatch> says how well individual args match to the params
    vector<pair<ScopeValue, vector<TypeMatch>>> viable_candidates;

    for (const auto &[decl, type]: candidates) {
        cpm::FunctionType *func_type = cpm::function_ty(type);
        assert(func_type);
        const std::vector<cpm::Type *> &params = func_type->getParams();
        const auto &def_args = funcs_def_args[decl];
        // too many arguments, no vararg
        if (arg_count > params.size() && !func_type->isVararg())
            continue;
        // too few arguments, not enough default arguments
        if (arg_count < params.size() && def_args[arg_count] == nullptr)
            continue;
        bool all_args_pass = true;
        vector<TypeMatch> arg_matches;
        for (size_t i = 0; i < arg_count && i < params.size(); i++) {
            TypeMatch match = implicitly_convertible(arg_types[i], params[i]);
            if (match == NONE) {
                all_args_pass = false;
                break;
            }
            arg_matches.push_back(match);
        }
        if (!all_args_pass)
            continue;
        viable_candidates.emplace_back(ScopeValue{decl, func_type}, std::move(arg_matches));
    }
    if (viable_candidates.empty())
        error("no matching function for call of '" + func_name + "'", node);
    // select the best match
    // graph problem: in a directed graph, is there a node which receives an edge from every other
    //                node, and does not have any outgoing edges?
    size_t best_index = 0;
    for (size_t i = 1; i < viable_candidates.size(); i++) {
        if (better_or_same_match(viable_candidates[i].second, viable_candidates[best_index].second))
            best_index = i;
    }
    for (size_t i = 0; i < viable_candidates.size(); i++) {
        if (i == best_index)
            continue;
        const vector<TypeMatch> &best = viable_candidates[best_index].second;
        const vector<TypeMatch> &contender = viable_candidates[i].second;
        // 'contender' is as good as 'best', or they are incomparable
        if (better_or_same_match(contender, best) || !better_or_same_match(best, contender))
            error("ambiguous call", node);
    }

    return viable_candidates[best_index].first;
}

SemanticChecker::TypeMatch
SemanticChecker::implicitly_convertible(cpm::Type *start, cpm::Type *dest) {
    if (const_unqualified_type(start) == const_unqualified_type(dest))
        return EXACT;
    else if (const_stronger_elem_type(dest, start))
        return CONST;
    cpm::PointerType *start_ptr = cpm::pointer_ty(start),
            *dest_ptr = cpm::pointer_ty(dest);
    cpm::SimpleType *start_st = cpm::simple_ty(start),
            *dest_st = cpm::simple_ty(dest);
    if (
            // 'T*' -> 'void*'
            (start_ptr && dest_ptr && is_void(dest_ptr->getElemType())) ||
            // 'nullptr_t' -> 'T*'
            (dest_ptr && start_st && start_st->getTypeId() == "nullptr_t") ||
            // to bool
            (is_bool(dest_st) && (start_ptr || is_numerical(start_st))) ||
            // integral promotions
            (is_integral(start_st) && is_integral(dest_st)) ||
            // int -> double
            (is_int(start_st) && is_double(dest_st)) ||
            // double -> int
            (is_double(start_st) && is_int(dest_st))
            )
        return CONVERSION;

    return NONE;
}

bool
SemanticChecker::better_or_same_match(const vector<TypeMatch> &m1, const vector<TypeMatch> &m2) {
    const size_t len = std::min(m1.size(), m2.size());
    for (size_t i = 0; i < len; i++)
        if (m1[i] > m2[i])
            return false;

    // variadic arguments are always worse (list with less elements taps into variadic)
    return m1.size() >= m2.size();
}

SemanticChecker::Value SemanticChecker::operator()(ast::DefaultArgExpr &node) {
    // use the value that was saved
    return def_arg_vals.at(node.expr);
}

void SemanticChecker::class_first_pass(ast::ClassDef &node) {
    string class_name = node.head->name;
    if (classes.contains(class_name))
        error("class " + class_name + " has already been declared", node);
    defined_class = class_name;
    classes[defined_class] = make_unique<Class>(current_scope);
    current_scope = classes[defined_class].get();

    // set up starting access
    if (node.head->key == ast::Struct)
        current_access = ast::PUBLIC;
    else if (node.head->key == ast::Class)
        current_access = ast::PRIVATE;
    else
        assert(false);

    // visit body
    if (node.body)
        class_first_pass(*node.body.value());
}

void SemanticChecker::class_first_pass(ast::MemberSpecification &node) {
    // first pass the class for fields, then declare methods
    // note: this defacto splits this into two passes, that would be ideal
    //       to do
    vector<pair<ast::FuncDef *, ast::AccessModifier>> methods;
    for (ast::node_ptr<ast::MemberSpecElem> &ms: node.list) {
        if (ast::AccessModifier *am = get_if<ast::AccessModifier>(ms.get()))
            current_access = *am;
        else if (ast::MemberDeclaration *md = get_if<ast::MemberDeclaration>(ms.get())) {
            pair<ast::FuncDef *, ast::AccessModifier> poss_method = class_first_pass(*md);
            if (poss_method.first != nullptr)
                methods.push_back(poss_method);
        } else
            assert(false);
    }
    // process the methods
    for (const auto &[func_def, am]: methods) {
        current_access = am;
        class_first_pass(*func_def);
    }
}

std::pair<ast::FuncDef *, ast::AccessModifier>
SemanticChecker::class_first_pass(ast::MemberDeclaration &node) {
    if (auto *mdl = get_if<ast::MemberDeclaratorList>(&node)) {
        for (auto &md: mdl->decls)
            class_first_pass(*md);
        return {nullptr, current_access};
    }
    // for function definitions, return them to be processed later
    else if (ast::FuncDef *fd = get_if<ast::FuncDef>(&node))
        return {fd, current_access};
    assert(false && "unimplemented MemberDeclaration case");
}

void SemanticChecker::class_first_pass(ast::MemberDeclarator &node) {
    // handle methods declarations
    if (cpm::function_ty(node.type)) {
        error("class methods cannot be forward declared", node);
    }
    // this assumes that functions are handled separately, so function overloading won't
    // trigger this
    assert(!cpm::function_ty(node.type));
    if (classes[defined_class]->contains(node.id))
        error("name '" + node.id + "' already exists in class scope", node);

    // for class fields, the class type itself is incomplete
    cpm::SimpleType *class_type = getSimpleType(defined_class, false);
    incomplete_types.insert(class_type);
    process(node);
    incomplete_types.erase(class_type);

    // add the field to class scope
    classes[defined_class]->addValue(node.id, node.type, &node, current_access);
}

void SemanticChecker::class_first_pass(ast::FuncDef &node) {
    ast::FunctionDecl *func_decl = node.declarator.get();
    add_this_to_func_decl(func_decl);
    // only declare the method, don't process default arguments or body yet
    declareFunction(func_decl, false);
}

SemanticChecker::Value SemanticChecker::operator()(ast::MemberAccessExpr &node) {
    Value val = process(node.object);
    cpm::SimpleType *class_type = get_underlying_type(val, node.ptr_access, node);
    if (node.ptr_access)
        node.object = convert_to_rval(std::move(node.object), val, val.type, node);
    else if (!node.ptr_access && val.valtype == RValue)
        error("member access of " + val.str(), node);

    string class_name = class_type->getTypeId();
    string member_name = node.member;
    assert(classes.contains(class_name));
    Class *class_scope = classes[class_name].get();
    if (!class_scope->contains(member_name))
        error("member access to unknown member '" + node.member + "'", node);
    vector<ScopeValue> vals = class_scope->getValues(member_name, false);
    assert(!vals.empty());
    // check that this is not a function
    if (cpm::function_ty(vals.at(0).type))
        error("member function accessed in a different context than call", node);

    // limit private access outside the class
    if (!inside_scope(class_scope))
        if (class_scope->is_private(vals[0].decl))
            error("cannot access private member '" + member_name + "'", node);

    if (class_type->isConst())
        return {const_type(vals[0].type), LValue};
    return {vals[0].type, LValue};
}

void SemanticChecker::class_second_pass(ast::ClassDef &node) {
    // reset starting access
    if (node.head->key == ast::Struct)
        current_access = ast::PUBLIC;
    else if (node.head->key == ast::Class)
        current_access = ast::PRIVATE;
    else
        assert(false);

    if (node.body.has_value())
        class_second_pass(*node.body.value());
}

void SemanticChecker::class_second_pass(ast::MemberSpecification &node) {
    for (ast::node_ptr<ast::MemberSpecElem> &ms: node.list) {
        if (ast::AccessModifier *am = get_if<ast::AccessModifier>(ms.get()))
            current_access = *am;
        else if (ast::MemberDeclaration *md = get_if<ast::MemberDeclaration>(ms.get()))
            class_second_pass(*md);
        else
            assert(false);
    }
}

void SemanticChecker::class_second_pass(ast::MemberDeclaration &node) {
    if (get_if<ast::MemberDeclaratorList>(&node))
        // this assumes that method declarations do nothing in the second pass
        return;
    else if (auto *fd = get_if<ast::FuncDef>(&node))
        return class_second_pass(*fd);
    assert(false && "unimplemented MemberDeclaration case");
}

void SemanticChecker::class_second_pass(ast::FuncDef &node) {
    // ideally we would check that the function has already been
    // declared in the scope
    process(node);
}

void SemanticChecker::operator()(ast::ClassDef &node) {
    Scope *parent = current_scope;
    // this sets current_scope to the new class scope
    class_first_pass(node);
    class_second_pass(node);

    // reset values
    current_scope = parent;
    defined_class = "";
}

SemanticChecker::Value SemanticChecker::operator()(ast::ThisExpr &node) {
    // we're not inside a class or inside a method
    if (defined_class.empty() || curr_ret_type == nullptr)
        error("cannot use 'this' here", node);
    // devnote: if const methods are added, this will have to be updated
    return {getPointerType(getSimpleType(defined_class, false), true), RValue};
}

bool SemanticChecker::inside_scope(Scope *scope) {
    Scope *s = current_scope;
    while (s) {
        if (scope == s)
            return true;
        s = s->getParent();
    }
    return false;
}

cpm::Type *SemanticChecker::const_type(cpm::Type *type) {
    assert(!cpm::function_ty(type));
    // arrays cannot be const, get to the underlying pointer/simple type
    while (cpm::ArrayType *at = cpm::array_ty(type))
        type = at->getElemType();
    if (cpm::PointerType *pt = cpm::pointer_ty(type)) {
        if (!pt->isConst())
            type = getPointerType(pt->getElemType(), true);
    } else if (cpm::SimpleType *st = cpm::simple_ty(type)) {
        if (!st->isConst())
            type = getSimpleType(st->getTypeId(), true);
    }

    return type;
}

// refactor: make a struct of the three types, and make the return of this std::optional
std::tuple<cpm::Type *, cpm::Type *, cpm::Type *>
SemanticChecker::conversions_for_bin_op(cpm::Type *lhs, cpm::Type *rhs, ast::BinaryOp op) {
    cpm::Type *bool_ty = getBoolType();
    cpm::Type *int_ty = getIntType();
    cpm::Type *common_type = determine_common_type(lhs, rhs);
    // for logical, convert both sides to 'bool'
    if (ast::is_logical_op(op)) {
        return {bool_ty, bool_ty, bool_ty};
    }
    // for bit-wise and %, both sides must be integral
    else if (ast::is_bit_op(op) || op == ast::Mod) {
        if (is_integral(lhs) && is_integral(rhs)) {
            // if size_t arrives, change this
            return {int_ty, int_ty, int_ty};
        }
    }
    // for comparison, their shared type must be numerical or pointer
    else if (ast::is_comp_op(op)) {
        if (is_numerical(common_type) || cpm::pointer_ty(common_type))
            return {common_type, common_type, bool_ty};
    }
    // for arithmetic, one type must be convertible to other, and for
    // '+' and '-', it can be ptr and integral
    else if (ast::is_arithmetic_op(op)) {
        // pointer arithmetic only works when the pointee is complete
        PointerType *lhs_ptr = pointer_to_complete_type(lhs);
        PointerType *rhs_ptr = pointer_to_complete_type(rhs);
        if (lhs_ptr && rhs_ptr)
            if (op == ast::Minus)
                return {common_type, common_type, int_ty};
        if (lhs_ptr && is_integral(rhs))
            if (op == ast::Plus || op == ast::Minus)
                return {lhs, int_ty, lhs};
        if (is_numerical(lhs) && is_numerical(rhs))
            return {common_type, common_type, common_type};
    }
    return {nullptr, nullptr, nullptr};
}

SemanticChecker::Value SemanticChecker::operator()(ast::ImplicitTypeCastExpr &node) {
    return {node.dest_ty, RValue};
}

SemanticChecker::Value SemanticChecker::operator()(ast::LValToRValExpr &node) {
    Value val = process(node.val);
    assert(val.valtype == LValue);
    return {val.type, RValue};
}

void SemanticChecker::run(ast::TranslationUnit &node) {
    process(node);
}

Scope *SemanticChecker::addScope(Scope *scope) {
    if (!scope) {
        scopes.push_back(make_unique<Scope>(current_scope));
        scope = scopes.back().get();
    }
    scope->setParent(current_scope);
    return current_scope = scope;
}

void SemanticChecker::dropScope() {
    assert(current_scope);
    assert(current_scope->getParent());
    current_scope = current_scope->getParent();
}

void SemanticChecker::error(const string &msg, const ast::Node &node) {
    string resp = "line " + node.src_info.str() + ": error: " + msg;
    resp += '\n' + context.getInputByLines().at(node.src_info.line_no);
    throw std::runtime_error(resp);
}

void SemanticChecker::compiler_error(const string &msg, const ast::Node &node) {
    string resp = "line " + node.src_info.str() + ": compiler error: " + msg;
    resp += '\n' + context.getInputByLines().at(node.src_info.line_no);
    throw std::runtime_error(resp);
}

void SemanticChecker::warning(const string &msg, const ast::Node &node) {
    string resp = "line " + node.src_info.str() + ": warning: " + msg;
    resp += '\n' + context.getInputByLines().at(node.src_info.line_no);
    warning_os << resp << '\n';
}

cpm::SimpleType *SemanticChecker::common_type(cpm::SimpleType *s1, cpm::SimpleType *s2) {
    if (is_nullptr(s1) && is_nullptr(s2))
        return getNullptrType();
    if (!is_numerical(s1) || !is_numerical(s2))
        return nullptr;
    if (is_double(s1) && is_double(s2))
        return getDoubleType();
    if ((is_double(s1) && is_int(s2)) || (is_int(s1) && is_double(s2)))
        return getDoubleType();
    // double with integral type that's not 'int', forbidden
    if (is_double(s1) || is_double(s2))
        return nullptr;

    // this should be true after all the previous checks
    assert(is_integral(s1) && is_integral(s2));
    const std::string &str1 = s1->getTypeId(), &str2 = s2->getTypeId();
    if (str1 == "int" || str2 == "int")
        return getIntType();
    else if (str1 == "char" || str2 == "char")
        return getCharType();
    else if (str1 == "bool" || str2 == "bool")
        return getBoolType();

    assert(false && "unexpected integral types");
}

cpm::Type *SemanticChecker::determine_common_type(cpm::Type *t1, cpm::Type *t2) {
    if (t1 == t2)
        return t1;
    if (const_unqualified_type(t1) == const_unqualified_type(t2))
        return const_unqualified_type(t1);

    TypeMatch t1_to_t2 = implicitly_convertible(t1, t2);
    TypeMatch t2_to_t1 = implicitly_convertible(t2, t1);
    // t1 can be converted to t2 but not vice-versa, return t2
    if (t1_to_t2 != NONE && t2_to_t1 == NONE)
        return t2;
    // t2 can be converted to t1 but not vice-versa, return t1
    if (t2_to_t1 != NONE && t1_to_t2 == NONE)
        return t1;

    cpm::SimpleType *s1 = cpm::simple_ty(t1),
            *s2 = cpm::simple_ty(t2);
    cpm::PointerType *p1 = cpm::pointer_ty(t1),
            *p2 = cpm::pointer_ty(t2);
    if (s1 && s2)
        return common_type(s1, s2);
    else if (p1 && p2)
        return common_type(p1, p2);

    return nullptr;
}

cpm::PointerType *SemanticChecker::common_type(cpm::PointerType *p1, cpm::PointerType *p2) {
    if (p1 == p2)
        return p1;
    else if (similar_types(p1, p2))
        return cpm::pointer_ty(common_type_rec(p1, p2));
    return nullptr;
}

cpm::Type *SemanticChecker::common_type_rec(cpm::Type *t1, cpm::Type *t2) {
    if (t1 == t2)
        return t1;

    if (SimpleType *s1 = simple_ty(t1), *s2 = simple_ty(t2); s1 && s2) {
        if (s1->getTypeId() != s2->getTypeId())
            return nullptr;
        return getSimpleType(s1->getTypeId(), s1->isConst() || s2->isConst());
    } else if (PointerType *p1 = pointer_ty(t1), *p2 = pointer_ty(t2); p1 && p2) {
        t1 = p1->getElemType();
        t2 = p2->getElemType();
        // always make the pointer type const, to prevent 'const Foo**' and 'Foo**' issues
        return getPointerType(common_type_rec(t1, t2), true);
    } else if (ArrayType *a1 = array_ty(t1), *a2 = array_ty(t2); a1 && a2) {
        assert(a1->getSize().has_value() && a2->getSize().has_value());
        // not sure if this won't trigger optional bad access, because we're
        // reaccessing the value through a method
        if (a1->getSize().value() != a2->getSize().value())
            return nullptr;
        t1 = a1->getElemType();
        t2 = a2->getElemType();
        return getArrayType(common_type_rec(t1, t2), a1->getSize().value());
    } else
        return nullptr;
}

cpm::SimpleType *
SemanticChecker::get_underlying_type(SemanticChecker::Value val, bool ptr_access,
                                     const ast::Node &node) {
    cpm::Type *underlying_type;
    if (ptr_access) {
        cpm::PointerType *ptr = cpm::pointer_ty(val.type);
        if (!ptr)
            error("pointer access through " + val.str(), node);
        underlying_type = ptr->getElemType();
    } else
        underlying_type = val.type;

    cpm::SimpleType *s = cpm::simple_ty(underlying_type);
    if (!s || !classes.contains(s->getTypeId()))
        error("accessed object is not of class or struct type: " + cpm::to_string(underlying_type),
              node);

    return s;
}

SemanticChecker::Value SemanticChecker::operator()(ast::ImplicitThisExpr &) {
    assert(!defined_class.empty());
    // change this with const methods
    return {getPointerType(getSimpleType(defined_class, false), true), RValue};
}

ast::node_ptr<ast::Expr>
SemanticChecker::convert_to_rval(ast::node_ptr<ast::Expr> val, Value from, cpm::Type *dest,
                                 const ast::Node &node, bool throw_error) {

    vector<cpm::Type *> types;

    ast::node_ptr<ast::Expr> rval;
    cpm::Type *rval_ty;

    /* 1) obtaining rvalue */
    // rvalue already
    if (from.valtype == RValue) {
        rval = std::move(val);
        rval_ty = const_unqualified_type(from.type);
    }
    // lvalue to rvalue
    else if (cpm::pointer_ty(from.type) || cpm::simple_ty(from.type)) {
        rval = ast::make_node<ast::LValToRValExpr, ast::Expr>(node.src_info, std::move(val));
        rval_ty = const_unqualified_type(from.type);
    }
    // note: array to ptr decay already took place when 'val' was visited
    else
        return convert_error(from, dest, node, throw_error);

    /* 2) seeing if type conversion is possible */
    TypeMatch am = implicitly_convertible(rval_ty, dest);
    switch (am) {
        case EXACT:
        case CONST:
            return rval;
        case CONVERSION:
            return ast::make_node<ast::ImplicitTypeCastExpr, ast::Expr>(node.src_info,
                                                                        std::move(rval), dest);
        case NONE:
        default:
            return convert_error(from, dest, node, throw_error);
    }
}

cpm::Type *SemanticChecker::const_unqualified_type(cpm::Type *t) {
    if (auto *st = cpm::simple_ty(t))
        return getSimpleType(st->getTypeId(), false);
    else if (auto *pt = cpm::pointer_ty(t))
        return getPointerType(pt->getElemType(), false);
    return t;
}

bool SemanticChecker::const_stronger_elem_type(cpm::Type *t1, cpm::Type *t2) {
    cpm::Type *e1 = nullptr, *e2 = nullptr;
    if (cpm::PointerType *p1 = cpm::pointer_ty(t1), *p2 = cpm::pointer_ty(t2);
            p1 && p2) {
        e1 = p1->getElemType(), e2 = p2->getElemType();
    } else if (cpm::ArrayType *a1 = cpm::array_ty(t1), *a2 = cpm::array_ty(t2);
            a1 && a2) {
        e1 = a1->getElemType(), e2 = a2->getElemType();
    } else
        return false;

    return is_const(e1) && !is_const(e2) &&
           const_unqualified_type(e1) == const_unqualified_type(e2);
}

SemanticChecker::Value SemanticChecker::operator()(ast::ArrToPtrExpr &node) {
    Value array = process(node.arr_expr, false);
    cpm::ArrayType *array_ty = cpm::array_ty(array.type);
    assert(array_ty);
    assert(array.valtype == LValue);

    return {getPointerType(array_ty->getElemType(), false), RValue};
}

SemanticChecker::Value
SemanticChecker::operator()(ast::node_ptr<ast::Expr> &node_ptr, bool array_to_ptr) {
    // before visiting the expression, check if it's an implicit class member access (to a field or a method)
    const ast::IdExpr *id_expr = nullptr;
    ast::node_ptr<ast::Expr> *replaced_expr = nullptr;
    // implicit field access
    if ((id_expr = get_if<ast::IdExpr>(node_ptr.get()))) {
        replaced_expr = &node_ptr;
    }
    // implicit method access
    else if (ast::CallExpr *call_expr = get_if<ast::CallExpr>(node_ptr.get())) {
        if ((id_expr = get_if<ast::IdExpr>(call_expr->called_func.get())))
            replaced_expr = &(call_expr->called_func);
    }
    if (id_expr)
        if (refers_to_class_member(id_expr->id)) {
            // check if a member is used in default argument
            if (current_scope == classes[defined_class].get())
                error("cannot use non-static member '" + id_expr->id + "' here", *id_expr);
            // replace the original expression with ImplicitThis access
            *replaced_expr = ast::make_node<ast::MemberAccessExpr, ast::Expr>(
                    id_expr->src_info,
                    ast::make_node<ast::ImplicitThisExpr, ast::Expr>(id_expr->src_info),
                    true,
                    id_expr->id);
        }

    // check for sizeof expr before visiting node_ptr,
    // so that array to pointer decay doesn't take place on the operand;
    // remake the node into SizeofTypeExpr
    if (auto *unary_expr = get_if<ast::UnaryExpr>(node_ptr.get())) {
        if (unary_expr->op == ast::Sizeof) {
            Type *sizeof_expr_type = process(unary_expr->expr, false).type;
            node_ptr = ast::make_node<ast::SizeofTypeExpr, ast::Expr>(
                    unary_expr->src_info,
                    sizeof_expr_type
            );
        }
    }

    // visit the expression
    Value val = std::visit(*this, *node_ptr);

    // array to pointer decay;
    //      &, sizeof and init by string literal have to prevent/undo this
    if (array_to_ptr) {
        if (auto array_ty = cpm::array_ty(val.type);
                array_ty && val.valtype == LValue) {
            // note: the source info line 0 should not cause problems (at least here) because ArrToPtrExpr
            //       does not throw errors, thus the line 0 should never be displayed in error reporting
            node_ptr = ast::make_node<ast::ArrToPtrExpr, ast::Expr>(ast::SourceInfo(),
                                                                    std::move(node_ptr));
            val = {getPointerType(array_ty->getElemType(), false), RValue};
        }
    }

    return val;
}

bool SemanticChecker::refers_to_class_member(const std::string &id) {
    return !defined_class.empty() &&
           // constructors are not considered members
           id != defined_class &&
           classes[defined_class].get() == current_scope->getValueScope(id);
}

SemanticChecker::Value SemanticChecker::operator()(ast::CastExpr &node) {
    cpm::Type *dest_ty = node.type;
    Value val = process(node.expr);
    node.expr = convert_to_rval(std::move(node.expr), val, val.type, node);
    if (!explicitly_convertible(val.type, dest_ty))
        error("cannot cast value of type " + cpm::to_string(val.type) + " to type " +
              cpm::to_string(dest_ty), node);

    node.type = dest_ty;
    return {dest_ty, RValue};
}

bool SemanticChecker::explicitly_convertible(cpm::Type *start_ty, cpm::Type *dest_ty) {
    return implicitly_convertible(start_ty, dest_ty) != NONE ||
           (cpm::pointer_ty(start_ty) && cpm::pointer_ty(dest_ty));
}

SemanticChecker::Value SemanticChecker::operator()(ast::SizeofTypeExpr &node) {
    cpm::Type *type = node.type;
    if (incomplete_type(type) || cpm::function_ty(type))
        error("sizeof invalid type " + cpm::to_string(type), node);
    node.type = type;
    // for now, the result of sizeof will be int
    return {getSizeofType(), RValue};
}

cpm::Type *SemanticChecker::getSizeofType() {
    return getIntType(true);
}

bool SemanticChecker::incomplete_type(cpm::Type *type) {
    return incomplete_types.contains(const_unqualified_type(type));
}

void SemanticChecker::add_this_to_func_decl(ast::FunctionDecl *func_decl) {
    assert(!defined_class.empty());
    cpm::FunctionType *orig_func_type = cpm::function_ty(func_decl->type);
    assert(orig_func_type);

    cpm::PointerType *this_type = getPointerType(getSimpleType(defined_class, false), true);
    vector<cpm::Type *> orig_params = orig_func_type->getParams();

    // change FunctionType
    vector<cpm::Type *> param_types = {this_type};
    param_types.insert(param_types.end(), orig_params.begin(), orig_params.end());
    func_decl->type = getFunctionType(orig_func_type->getRetType(), std::move(param_types),
                                      orig_func_type->isVararg());
    // add a new Param
    ast::node_ptr<ast::Param> this_param = ast::make_node<ast::Param>(
            func_decl->src_info,
            // important: LLBuilder depends on the name being "this"
            ast::make_node<ast::Decl>(func_decl->src_info, this_type, "this"),
            std::nullopt);
    func_decl->params.insert(func_decl->params.begin(), std::move(this_param));
}

void SemanticChecker::operator()(ast::Decl &node) {
    validate_type(node.type, node);
    // check that the type is not incomplete
    // note: this test might be too strong, there might be
    //       valid cases of declarators with incomplete types
    //       that I haven't thought of. In that case, this check
    //       will have to be replicated where it's needed (function
    //       variables, class members, ...?).
    if (incomplete_type(node.type))
        error("declarator '" + node.id + "' has incomplete type: " +
              to_string(node.type), node);
}

void SemanticChecker::validate_type(cpm::Type *type, const ast::Node &node) {
    if (valid_types_cache.contains(type))
        return;
    else if (auto *st = simple_ty(type)) {
        if (!type_exists(st->getTypeId()))
            error("unknown type: " + to_string(st), node);
    } else if (auto *pt = pointer_ty(type)) {
        cpm::Type *elem_type = pt->getElemType();
        if (cpm::function_ty(elem_type))
            error("pointer to function type is not allowed", node);
        validate_type(elem_type, node);
    } else if (auto *at = array_ty(type)) {
        Type *elem_type = at->getElemType();
        // array elem type cannot be a function, must be complete, and the array
        // must have known size
        if (function_ty(elem_type))
            error("array of functions is not allowed", node);
        if (!at->getSize().has_value())
            error("array without defined size", node);
        if (incomplete_type(elem_type))
            error("array element type cannot be incomplete: " + to_string(elem_type), node);
        validate_type(elem_type, node);
    } else if (auto *ft = function_ty(type)) {
        // return type
        Type *ret_type = ft->getRetType();
        if (!is_void(ret_type) && incomplete_type(ret_type))
            error("incomplete function return type", node);
        if (array_ty(ret_type))
            error("function cannot return array", node);
        if (function_ty(ret_type))
            error("function cannot return function", node);
        validate_type(ret_type, node);
        // params
        for (Type *p: ft->getParams()) {
            if (incomplete_type(p))
                error("parameter has incomplete type: " + to_string(p), node);
            if (array_ty(p))
                error("parameter is of array type", node);
            if (function_ty(p))
                error("parameter is of function type", node);
            validate_type(p, node);
        }
        // vararg
        if (ft->getParams().empty() && ft->isVararg())
            error("vararg function without any parameters", node);
    } else
        assert(false && "unexpected type");

    valid_types_cache.insert(type);
}

void SemanticChecker::operator()(ast::Stmt &node) {
    return std::visit(*this, node);
}

void SemanticChecker::operator()(ast::Declaration &node) {
    return std::visit(*this, node);
}

void SemanticChecker::operator()(ast::ForInitStmt &node) {
    return std::visit(*this, node);
}

bool SemanticChecker::is_floating(cpm::Type *type) {
    SimpleType *simple_ty = cpm::simple_ty(type);
    return simple_ty && is_floating(simple_ty);
}

bool SemanticChecker::is_floating(cpm::SimpleType *simple_ty) {
    return simple_ty && simple_ty->getTypeId() == "double";
}

cpm::SimpleType *SemanticChecker::getDoubleType(bool is_const) {
    return getSimpleType("double", is_const);
}

cpm::SimpleType *SemanticChecker::getCharType(bool is_const) {
    return getSimpleType("char", is_const);
}

bool SemanticChecker::similar_types(Type *t1, Type *t2) {
    while (t1 && t2) {
        if (t1 == t2)
            return true;
        if (SimpleType *s1 = simple_ty(t1), *s2 = simple_ty(t2); s1 && s2) {
            return s1->getTypeId() == s2->getTypeId();
        } else if (PointerType *p1 = pointer_ty(t1), *p2 = pointer_ty(t2); p1 && p2) {
            t1 = p1->getElemType();
            t2 = p2->getElemType();
            continue;
        } else if (ArrayType *a1 = array_ty(t1), *a2 = array_ty(t2); a1 && a2) {
            assert(a1->getSize().has_value() && a2->getSize().has_value());
            // not sure if this won't trigger optional bad access, because we're
            // reaccessing the value through a method
            if (a1->getSize().value() != a2->getSize().value())
                return false;
            t1 = a1->getElemType();
            t2 = a2->getElemType();
            continue;
        }
        // function types are only similar if they are literally the same type, see
        // https://en.cppreference.com/w/cpp/language/implicit_conversion#Similar_types
        else
            return false;
    }
    return false;
}

void SemanticChecker::add_builtin_functions(ast::TranslationUnit &tu) {
    SimpleType *int_ty = getIntType();
    PointerType *const_char_ptr_ty = getPointerType(getCharType(true), false);
    PointerType *char_ptr_ty = getPointerType(getCharType(false), false);
    PointerType *void_ptr_ty = getPointerType(getVoidType(), false);

    vector<ast::node_ptr<ast::Declaration>> funcs_declarations;

    // printf
    vector<ast::node_ptr<ast::Param>> printf_params;
    printf_params.push_back(
            make_param(const_char_ptr_ty, "format")
    );
    ast::node_ptr<ast::Declaration> printf = create_func_declaration_node(
            "printf", int_ty, std::move(printf_params), true
    );

    // scanf
    vector<ast::node_ptr<ast::Param>> scanf_params;
    scanf_params.push_back(
            make_param(const_char_ptr_ty, "format")
    );
    ast::node_ptr<ast::Declaration> scanf = create_func_declaration_node(
            "scanf", int_ty, std::move(scanf_params), true
    );

    // malloc
    vector<ast::node_ptr<ast::Param>> malloc_params;
    malloc_params.push_back(
            make_param(int_ty, "bytes")
    );
    ast::node_ptr<ast::Declaration> malloc = create_func_declaration_node(
            "malloc", void_ptr_ty, std::move(malloc_params), false
    );

    // free
    vector<ast::node_ptr<ast::Param>> free_params;
    free_params.push_back(
            make_param(void_ptr_ty, "ptr")
    );
    ast::node_ptr<ast::Declaration> free = create_func_declaration_node(
            "free", getVoidType(), std::move(free_params), false
    );

    // sprintf
    vector<ast::node_ptr<ast::Param>> sprintf_params;
    sprintf_params.push_back(
            make_param(char_ptr_ty, "dest"));
    sprintf_params.push_back(
            make_param(const_char_ptr_ty, "format")
    );
    ast::node_ptr<ast::Declaration> sprintf = create_func_declaration_node(
            "sprintf", int_ty, std::move(sprintf_params), true
    );

    // sscanf
    vector<ast::node_ptr<ast::Param>> sscanf_params;
    sscanf_params.push_back(
            make_param(const_char_ptr_ty, "src"));
    sscanf_params.push_back(
            make_param(const_char_ptr_ty, "format")
    );
    ast::node_ptr<ast::Declaration> sscanf = create_func_declaration_node(
            "sscanf", int_ty, std::move(sscanf_params), true
    );

    // finish
    funcs_declarations.push_back(std::move(printf));
    funcs_declarations.push_back(std::move(scanf));
    funcs_declarations.push_back(std::move(malloc));
    funcs_declarations.push_back(std::move(free));
    funcs_declarations.push_back(std::move(sprintf));
    funcs_declarations.push_back(std::move(sscanf));

    tu.declars.insert(tu.declars.begin(),
                      make_move_iterator(funcs_declarations.begin()),
                      make_move_iterator(funcs_declarations.end()));
}

ast::node_ptr<ast::Declaration>
SemanticChecker::create_func_declaration_node(const string &func_name, cpm::Type *ret_type,
                                              std::vector<ast::node_ptr<ast::Param>> params,
                                              bool is_vararg) {
    ast::SourceInfo src_info;

    // get function type
    vector<Type *> param_types;
    for (const auto &p: params)
        param_types.push_back(p->declarator->type);
    FunctionType *func_type = getFunctionType(ret_type, param_types, is_vararg);
    // get function declarator
    auto func_decl = ast::make_node<ast::FunctionDecl>(
            src_info,
            func_type,
            func_name,
            std::move(params)
    );
    // get init declarator list
    vector<ast::node_ptr<ast::InitDeclarator>> init_declars;
    init_declars.push_back(
            ast::make_node<ast::InitDeclarator>(
                    src_info,
                    std::move(func_decl),
                    std::nullopt
            )
    );

    return ast::make_node<ast::SimpleDeclar, ast::Declaration>(
            src_info,
            std::move(init_declars)
    );
}

ast::node_ptr<ast::Param> SemanticChecker::make_param(Type *type, const std::string &name,
                                                      std::optional<ast::node_ptr<ast::Expr>> def_val,
                                                      const ast::SourceInfo &src_info) {
    return ast::make_node<ast::Param>(
            src_info,
            ast::make_node<ast::Decl>(
                    src_info,
                    type,
                    name
            ),
            std::move(def_val)
    );
}

void SemanticChecker::operator()(ast::FunctionDecl &node, const bool process_def_args) {
    // check that the type is valid
    if (!function_ty(node.type))
        compiler_error("invalid function type: " + to_string(node.type), node);
    // run the standard declarator check
    process(dynamic_cast<ast::Decl &>(node));
    // check for parameter validity, name collisions and gaps in default arguments
    bool def_args_started = false;
    set<string> param_names;
    for (const auto &p: node.params) {
        // param validity
        process(*p->declarator);
        // params name collision
        const auto &p_name = p->declarator->id;
        if (param_names.contains(p_name))
            error("repeated parameter name: '" + p_name + "'", node);
        param_names.insert(p_name);
        // default arg gap
        if (def_args_started && !p->default_val.has_value())
            error("gap in default argument values", node);
        // handle default argument value
        if (p->default_val.has_value()) {
            def_args_started = true;
            if (process_def_args)
                p->default_val = convert_to_rval(std::move(p->default_val.value()),
                                                 p->declarator->type, node);
        }
    }
}

cpm::PointerType *SemanticChecker::pointer_to_complete_type(cpm::Type *type) {
    cpm::PointerType *pt = cpm::pointer_ty(type);
    if (pt && !incomplete_type(pt->getElemType()))
        return pt;
    return nullptr;
}

cpm::SimpleType *SemanticChecker::getNullptrType() {
    return getSimpleType("nullptr_t", false);
}


