#include "LLBuilder.h"

using namespace std;
using namespace cpm;

void LLBuilder::operator()(const ast::DeclarStmt &node) {
    codegen(*node.declaration);
}

void LLBuilder::operator()(const ast::ExprStmt &node) {
    if (node.expr.has_value())
        codegen(*node.expr.value());
}

void LLBuilder::operator()(const ast::BreakStmt &node) {
    // break_level=1 will result in break_bbs.back()
    llvm::BasicBlock *bb = break_bbs.at(break_bbs.size() - node.break_level);
    builder.CreateBr(bb);
}

void LLBuilder::operator()(const ast::ContinueStmt &node) {
    // continue_level=1 will result in continue_bbs.back()
    llvm::BasicBlock *bb = continue_bbs.at(continue_bbs.size() - node.continue_level);
    builder.CreateBr(bb);
}

void LLBuilder::operator()(const ast::ReturnStmt &node) {
    llvm::Function *curr_func = getCurrentFunction();
    check(curr_func);
    llvm::Type *ret_type = curr_func->getReturnType();
    if (node.expr.has_value()) {
        llvm::Value *ret_expr = codegen(*node.expr.value());
        if (ret_type != builder.getVoidTy())
            builder.CreateStore(ret_expr, ret_val);
    }
    builder.CreateBr(return_bb);
    // CreateRet instruction is called when visiting ast::FuncDef
}

void LLBuilder::operator()(const ast::CompoundStmt &node) {
    for (const auto &stmt: node.statements)
        codegen(*stmt);
}

void LLBuilder::operator()(const ast::DoWhileStmt &node) {
    llvm::Function *llvm_func = getCurrentFunction();
    check(llvm_func);
    string no = std::to_string(node.src_info.line_no);
    llvm::BasicBlock
            *body = llvm::BasicBlock::Create(context, "do_while.body_" + no, llvm_func),
            *cond = llvm::BasicBlock::Create(context, "do_while.cond_" + no),
            *end = llvm::BasicBlock::Create(context, "do_while.end_" + no);
    // save basic blocks for 'break' and 'continue'
    break_bbs.push_back(end);
    continue_bbs.push_back(cond);
    // generate body
    builder.CreateBr(body);
    builder.SetInsertPoint(body);
    codegen(*node.body);
    // the body might contain return, break or something -> in that case we don't
    // want to branch
    if (!builder.GetInsertBlock()->getTerminator())
        builder.CreateBr(cond);
    // generate condition
    llvm_func->getBasicBlockList().push_back(cond);
    builder.SetInsertPoint(cond);
    llvm::Value *cond_val = codegen(*node.cond);
    builder.CreateCondBr(cond_val, body, end);
    // generate end
    llvm_func->getBasicBlockList().push_back(end);
    builder.SetInsertPoint(end);

    // cleanup
    break_bbs.pop_back();
    continue_bbs.pop_back();
}

llvm::Function *LLBuilder::getCurrentFunction() {
    llvm::BasicBlock *current_bb = builder.GetInsertBlock();
    return current_bb ? current_bb->getParent() : nullptr;
}

void LLBuilder::operator()(const ast::ForStmt &node) {
    llvm::Function *curr_func = getCurrentFunction();
    check(curr_func);
    string no = std::to_string(node.src_info.line_no);
    // 'post_iter' and 'end' will be added to bb list later, for intuitive ir order where
    // post_iter is after body
    llvm::BasicBlock *preloop = llvm::BasicBlock::Create(context, "for.pre_" + no, curr_func),
            *cond = llvm::BasicBlock::Create(context, "for.cond_" + no, curr_func),
            *body = llvm::BasicBlock::Create(context, "for.body_" + no, curr_func),
            *post_iter = llvm::BasicBlock::Create(context, "for.post_" + no),
            *end = llvm::BasicBlock::Create(context, "for.end_" + no);
    // add bbs for break and continue
    break_bbs.push_back(end);
    continue_bbs.push_back(post_iter);
    // generate preloop
    builder.CreateBr(preloop);
    builder.SetInsertPoint(preloop);
    codegen(*node.initStmt);
    //      forInitStmt shouldn't be able
    //      to create a terminator, no need to check
    //      for that then
    builder.CreateBr(cond);
    // generate cond
    builder.SetInsertPoint(cond);
    if (node.cond.has_value()) {
        llvm::Value *cond_val = codegen(*node.cond.value());
        builder.CreateCondBr(cond_val, body, end);
    } else
        builder.CreateBr(body);
    // generate body
    builder.SetInsertPoint(body);
    codegen(*node.body);
    // the body might contain return, break or something -> in that case we don't
    // want to branch
    if (!builder.GetInsertBlock()->getTerminator())
        builder.CreateBr(post_iter);
    // generate post_iter
    curr_func->getBasicBlockList().push_back(post_iter);
    builder.SetInsertPoint(post_iter);
    if (node.post_iter.has_value())
        codegen(*node.post_iter.value());
    builder.CreateBr(cond);
    // generate end
    curr_func->getBasicBlockList().push_back(end);
    // if the 'end' block is not referred, delete it;
    // note: this (probably) means that the loop has empty condition
    // and a return inside the loop body
    if (llvm::pred_empty(end))
        end->eraseFromParent();
    else
        builder.SetInsertPoint(end);

    // cleanup
    break_bbs.pop_back();
    continue_bbs.pop_back();
}

llvm::Value *LLBuilder::operator()(const ast::BinaryExpr &node) {
    if (node.op == ast::LogicalAnd ||
        node.op == ast::LogicalOr)
        return create_shortcircuit(node);

    llvm::Value *lhs = codegen(*node.lhs);
    llvm::Value *rhs = codegen(*node.rhs);
    return create_binary_op(lhs, rhs, node.op);
}

llvm::Value *LLBuilder::create_binary_op(llvm::Value *lhs, llvm::Value *rhs, ast::BinaryOp op) {
    llvm::Type *lhs_ty = lhs->getType();
    llvm::Type *rhs_ty = rhs->getType();

    // integers
    if (lhs_ty->isIntegerTy() && rhs_ty->isIntegerTy()) {
        // llvm requires binary operands to be of the same type
        check(lhs_ty == rhs_ty);
        switch (op) {
            case ast::Plus:
                return builder.CreateAdd(lhs, rhs);
            case ast::Minus:
                return builder.CreateSub(lhs, rhs);
            case ast::Star:
                return builder.CreateMul(lhs, rhs);
            case ast::Div:
                return builder.CreateSDiv(lhs, rhs);
            case ast::Mod:
                return builder.CreateSRem(lhs, rhs);
            case ast::And:
                return builder.CreateAnd(lhs, rhs);
            case ast::Or:
                return builder.CreateOr(lhs, rhs);
            case ast::Caret:
                return builder.CreateXor(lhs, rhs);
            case ast::LeftShift:
                return builder.CreateShl(lhs, rhs);
            case ast::RightShift:
                return builder.CreateAShr(lhs, rhs);
            case ast::Greater:
                return builder.CreateICmpSGT(lhs, rhs);
            case ast::Less:
                return builder.CreateICmpSLT(lhs, rhs);
            case ast::GreaterEqual:
                return builder.CreateICmpSGE(lhs, rhs);
            case ast::LessEqual:
                return builder.CreateICmpSLE(lhs, rhs);
            case ast::Equal:
                return builder.CreateICmpEQ(lhs, rhs);
            case ast::NotEqual:
                return builder.CreateICmpNE(lhs, rhs);
                // LogicalAnd and LogicalOr are handled separately (shortcircuit)
            default:
                break;
        }
    }
    // doubles
    else if(lhs_ty->isDoubleTy() && rhs_ty->isDoubleTy()) {
        check(lhs_ty == rhs_ty);
        switch (op) {
            case ast::Plus:
                return builder.CreateFAdd(lhs, rhs);
            case ast::Minus:
                return builder.CreateFSub(lhs, rhs);
            case ast::Star:
                return builder.CreateFMul(lhs, rhs);
            case ast::Div:
                return builder.CreateFDiv(lhs, rhs);
            case ast::Mod:
                return builder.CreateFRem(lhs, rhs);
                // choose the 'O' version instead of 'U'; it's related to NaNs
            case ast::Greater:
                return builder.CreateFCmpOGT(lhs, rhs);
            case ast::Less:
                return builder.CreateFCmpOLT(lhs, rhs);
            case ast::GreaterEqual:
                return builder.CreateFCmpOGE(lhs, rhs);
            case ast::LessEqual:
                return builder.CreateFCmpOLE(lhs, rhs);
            case ast::Equal:
                return builder.CreateFCmpOEQ(lhs, rhs);
            case ast::NotEqual:
                return builder.CreateFCmpONE(lhs, rhs);
            default:
                break;
        }
    }
    // pointer indexing
    else if(lhs_ty->isPointerTy() && rhs_ty->isIntegerTy()) {
        llvm::Type *elem_type = lhs_ty->getPointerElementType();
        llvm::Value *index = nullptr;
        switch (op) {
            case ast::Plus:
                index = rhs;
                break;
            case ast::Minus:
                // make it 'lhs + (-rhs)'
                index = create_unary_minus(rhs);
                break;
            default:
                check(false, "unimplemented operator for pointer and integer");
        }
        return builder.CreateGEP(elem_type, lhs, index);
    }
    // pointer comparison and difference
    else if (lhs_ty->isPointerTy() && rhs_ty->isPointerTy()) {
        check(lhs_ty == rhs_ty);
        switch (op) {
            // use unsigned comparison for pointers
            case ast::Greater:
                return builder.CreateICmpUGT(lhs, rhs);
            case ast::Less:
                return builder.CreateICmpULT(lhs, rhs);
            case ast::GreaterEqual:
                return builder.CreateICmpUGE(lhs, rhs);
            case ast::LessEqual:
                return builder.CreateICmpULE(lhs, rhs);
            case ast::Equal:
                return builder.CreateICmpEQ(lhs, rhs);
            case ast::NotEqual:
                return builder.CreateICmpNE(lhs, rhs);
            case ast::Minus:
                // ptr_diff returns i64, we need to make it to 'int'
                return convert(builder.CreatePtrDiff(lhs_ty->getPointerElementType(),
                                                     lhs,
                                                     rhs),
                               types.at("int"));
            default:
                break;
        }
    }
    compiler_error("unhandled case in create_binary_op");
}

llvm::Value *LLBuilder::operator()(const ast::NullptrLiteral &) {
    return llvm::Constant::getNullValue(types.at("nullptr_t"));
}

llvm::Value *LLBuilder::operator()(const ast::IntLiteral &node) {
    return llvm::ConstantInt::get(types.at("int"), node.val, true);
}

llvm::Value *LLBuilder::operator()(const ast::AssignmentExpr &node) {
    check(node.lhs_type.has_value());
    // this is lvalue
    llvm::Value *lhs = codegen(*node.lhs);
    // this is rvalue, already converted to correct type if necessary
    llvm::Value *rhs = codegen(*node.rhs);
    llvm::Value *assigned_val = nullptr;
    if (node.op == ast::Assign)
        assigned_val = rhs;
    else {
        ast::BinaryOp op = ast::assign_op_to_binary_op(node.op);
        llvm::Value *lhs_rvalue = create_load(lhs);
        llvm::Type *lhs_op_type = get_llvm_type(node.lhs_type.value());
        llvm::Value *lhs_converted = convert(lhs_rvalue, lhs_op_type);
        llvm::Value *op_res = create_binary_op(lhs_converted, rhs, op);
        assigned_val = convert(op_res, lhs_rvalue->getType());
    }
    builder.CreateStore(assigned_val, lhs);

    // return the lvalue
    return lhs;
}

llvm::Value *LLBuilder::operator()(const ast::CommaExpr &node) {
    check(!node.expressions.empty());
    llvm::Value *res = nullptr;
    for (const auto &e: node.expressions)
        res = codegen(*e);
    return res;
}

llvm::Value *LLBuilder::create_shortcircuit(const ast::BinaryExpr &node) {
    check(node.op == ast::LogicalOr || node.op == ast::LogicalAnd);
    llvm::BasicBlock *lhs_bb = builder.GetInsertBlock();
    llvm::BasicBlock *rhs_bb = newBB("lazy_rhs");
    llvm::BasicBlock *end_bb = newBB("lazy_end");
    // generate lhs
    llvm::Value *lhs = codegen(*node.lhs);
    // lhs could've generated more basic blocks, try out '(a || b) || c'
    lhs_bb = builder.GetInsertBlock();
    // the shortcircuit logic is different for && and ||
    llvm::Value *eval_rhs = node.op == ast::LogicalAnd ? lhs : builder.CreateNot(lhs);
    builder.CreateCondBr(eval_rhs, rhs_bb, end_bb);
    // generate rhs
    builder.SetInsertPoint(rhs_bb);
    llvm::Value *rhs = codegen(*node.rhs);
    // rhs could've also generated more basic blocks
    rhs_bb = builder.GetInsertBlock();
    builder.CreateBr(end_bb);
    // generate end
    builder.SetInsertPoint(end_bb);
    llvm::PHINode *phi_node = builder.CreatePHI(builder.getInt1Ty(), 2);
    phi_node->addIncoming(lhs, lhs_bb);
    phi_node->addIncoming(rhs, rhs_bb);
    return phi_node;
}

void LLBuilder::operator()(const ast::IfStmt &node) {
    std::string line_no = std::to_string(node.src_info.line_no);
    llvm::Function *llvm_func = builder.GetInsertBlock()->getParent();
    llvm::BasicBlock
            // this block is unnecessary, but should improve ir readability
            *cond = llvm::BasicBlock::Create(context, "if.cond_" + line_no, llvm_func),
            *then = llvm::BasicBlock::Create(context, "if.body_" + line_no, llvm_func),
            *else_ = llvm::BasicBlock::Create(context, "if.else_" + line_no),
            *if_end = llvm::BasicBlock::Create(context, "if.end_" + line_no);

    // generate condition
    builder.CreateBr(cond);
    builder.SetInsertPoint(cond);
    llvm::Value *cond_val = codegen(*node.cond);
    builder.CreateCondBr(cond_val, then, else_);
    // generate then
    builder.SetInsertPoint(then);
    codegen(*node.body);
    // body can contain return, break or something
    if (!builder.GetInsertBlock()->getTerminator())
        builder.CreateBr(if_end);
    // generate else
    llvm_func->getBasicBlockList().push_back(else_);
    builder.SetInsertPoint(else_);
    if (node.else_body.has_value())
        codegen(*node.else_body.value());
    if (!builder.GetInsertBlock()->getTerminator())
        builder.CreateBr(if_end);

    llvm_func->getBasicBlockList().push_back(if_end);
    // if the 'end' block is not referred to by any other block,
    // delete it;
    // this probably means that there's a return, continue or break
    // inside both the body and the else_body
    if (llvm::pred_empty(if_end))
        if_end->eraseFromParent();
    else
        builder.SetInsertPoint(if_end);
}

void LLBuilder::operator()(const ast::WhileStmt &node) {
    string line_no = std::to_string(node.src_info.line_no);
    llvm::BasicBlock *cond = newBB("while.cond_" + line_no);
    llvm::BasicBlock *body = newBB("while.body" + line_no);
    llvm::BasicBlock *end = newBB("while.end" + line_no);

    break_bbs.push_back(end);
    continue_bbs.push_back(cond);

    // start by going to condition
    builder.CreateBr(cond);
    // generate condition
    builder.SetInsertPoint(cond);
    llvm::Value *cond_val = codegen(*node.cond);
    builder.CreateCondBr(cond_val, body, end);
    // generate body
    builder.SetInsertPoint(body);
    codegen(*node.body);
    // body can contain return, break or something
    if (!builder.GetInsertBlock()->getTerminator())
        builder.CreateBr(cond);
    // generate end
    builder.SetInsertPoint(end);
    // cleanup
    break_bbs.pop_back();
    continue_bbs.pop_back();
}

llvm::Value *LLBuilder::operator()(const ast::Condition &node) {
    llvm::Value *val = codegen(*node.expr);
    check(val->getType() == builder.getInt1Ty());
    return val;
}

llvm::Value *LLBuilder::operator()(const ast::ImplicitTypeCastExpr &node) {
    llvm::Value *val = codegen(*node.val);
    llvm::Type *type = get_llvm_type(node.dest_ty);
    return convert(val, type);
}

llvm::Value *LLBuilder::operator()(const ast::UnaryExpr &node) {
    llvm::Value *val = codegen(*node.expr);
    switch (node.op) {
        // we're holding lvalue, which is represented by pointer in llvm
        case ast::UnAnd:
        // we're holding rvalue of llvm pointer type
        case ast::UnStar:
        // unary plus is just the value
        case ast::UnPlus:
            return val;
        case ast::PlusPlus:
        case ast::MinusMinus: {
            llvm::Value *new_val = incr_decr(val, node.op == ast::PlusPlus);
            builder.CreateStore(new_val, val);
            // return the original lvalue
            return val;
        }
        case ast::UnMinus:
            return create_unary_minus(val);
        case ast::BitNot:
            // create xor with all ones
            return builder.CreateXor(val, llvm::ConstantInt::get(val->getType(), -1));
        case ast::Not:
            return builder.CreateNot(val);
        // sizeof as unary expression should've been converted to SizeofTypeExpr during
        // semantic analysis
        case ast::Sizeof:
            compiler_error("unary ast::Sizeof is unimplemented, expected to be "
                           "transformed into ast::SizeofTypeExpr");
        default:
            compiler_error("unimplemented unary op");
    }
}

llvm::Value *LLBuilder::incr_decr(llvm::Value *lvalue, bool incr) {
    llvm::Value *rvalue = create_load(lvalue);
    llvm::Value *one = rvalue->getType()->isPointerTy() ?
                       // incrementing pointer
                       builder.getInt32(1) :
                       // incrementing a number
                       llvm::ConstantInt::get(rvalue->getType(), 1);
    ast::BinaryOp op = incr ? ast::Plus : ast::Minus;

    return create_binary_op(rvalue, one, op);
}

llvm::Value *LLBuilder::operator()(const ast::PostIncrExpr &node) {
    llvm::Value *lvalue = codegen(*node.expr);
    // save the current value before incrementing
    llvm::Value *old_val = create_load(lvalue);
    // do the increment
    llvm::Value *new_val = incr_decr(lvalue, node.incr);
    builder.CreateStore(new_val, lvalue);
    // return the old value
    return old_val;
}

llvm::Value *LLBuilder::operator()(const ast::IdExpr &node) {
    check(node.var.has_value());
    const ast::Decl *ast_decl = node.var.value();
    return vals.at(ast_decl);
}

void LLBuilder::operator()(const ast::InitDeclarator &node) {
    const ast::Decl *decl = node.declarator.get();
    // function forward declarations are handled separately
    if (const auto *func_decl = dynamic_cast<const ast::FunctionDecl *>(decl)) {
        getFunction(*func_decl);
        return;
    }
    // variable declaration
    const string &id = decl->id;
    llvm::Type *type = get_llvm_type(decl->type);
    llvm::Value *val;
    // global scope
    if (!getCurrentFunction())
        val = new llvm::GlobalVariable(
                module,
                type,
                false,
                llvm::GlobalValue::ExternalLinkage,
                llvm::Constant::getNullValue(type),
                "global_" + id);
    // local scope
    else
        val = builder.CreateAlloca(type, nullptr, id);
    // save the value
    vals[decl] = val;

    if (node.initializer) {
        // global scope
        if (!getCurrentFunction())
            initialize_global_var(val, *node.initializer.value());
        // local scope
        else
            builder.CreateStore(codegen(*node.initializer.value()), val);
    }
}

void LLBuilder::operator()(const ast::FuncDef &node) {
    llvm::Function *func = getFunction(*node.declarator);
    llvm::Type *ret_ty = func->getReturnType();

    check(return_bb == nullptr);
    check(ret_val == nullptr);

    // get into the function body
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entry", func);
    builder.SetInsertPoint(entry);

    // prepare the return_bb and ret_val alloca for non-void functions
    return_bb = llvm::BasicBlock::Create(context, "return");
    if (!ret_ty->isVoidTy()) {
        ret_val = builder.CreateAlloca(ret_ty, nullptr, "ret_val");
        if (node.declarator->id == "main")
            // implicit return of 0
            builder.CreateStore(llvm::Constant::getNullValue(func->getReturnType()), ret_val);
    }

    // handle parameters
    const vector<ast::node_ptr<ast::Param>> &params = node.declarator->params;
    for (size_t i = 0; i < params.size(); i++) {
        const string &name = params[i]->declarator->id;
        llvm::Argument *arg = func->getArg(i);
        // set arg name in function header
        arg->setName(name);
        // 'this' is a prvalue expression, so it doesn't need an alloca
        if (i == 0 && name == "this") {
            this_rval = arg;
            continue;
        }

        // create alloca for the argument and store the initial value there
        llvm::AllocaInst *alloca = builder.CreateAlloca(arg->getType(), nullptr, name + ".addr");
        builder.CreateStore(arg, alloca);
        // save the alloca in vals so that it can be referred to in the function body
        vals[params[i]->declarator.get()] = alloca;
    }

    // generate body
    codegen(*node.body);
    // generate implicit return for main and void functions
    if (!builder.GetInsertBlock()->getTerminator())
        if (node.declarator->id == "main" || ret_ty->isVoidTy())
            builder.CreateBr(return_bb);

    // add return_bb to the end of function, and create the return instruction
    func->getBasicBlockList().push_back(return_bb);
    builder.SetInsertPoint(return_bb);
    if (ret_ty->isVoidTy())
        builder.CreateRetVoid();
    else {
        llvm::Value *load = create_load(ret_val);
        builder.CreateRet(load);
    }

    // cleanup
    return_bb = nullptr;
    ret_val = nullptr;
    this_lval = nullptr;
    this_rval = nullptr;
    builder.ClearInsertionPoint();
}

llvm::Function *LLBuilder::getFunction(const ast::FunctionDecl &node) {
    // this function has already been declared
    if (node.orig.has_value())
        return functions.at(node.orig.value());
        // second pass over the first declaration of a function
    else if (functions.contains(&node))
        return functions.at(&node);

    // this is the first declaration, create the function
    auto *func_type =
            llvm::dyn_cast<llvm::FunctionType>(get_llvm_type(node.type));
    check(func_type);
    string name = get_function_name(&node);
    llvm::Function *func = llvm::Function::Create(
            func_type,
            llvm::Function::ExternalLinkage,
            name,
            &module);

    // save the function for other declarations
    functions[&node] = func;

    // set parameter names with the first declaration
    for (size_t i = 0; i < node.params.size(); i++)
        func->getArg(i)->setName(node.params.at(i)->declarator->id);

    return func;
}

llvm::Value *LLBuilder::operator()(const ast::CharLiteral &node) {
    return llvm::ConstantInt::get(types.at("char"), node.c, true);
}

llvm::Value *LLBuilder::operator()(const ast::ThisExpr &) {
    return get_this_object();
}

llvm::Value *LLBuilder::operator()(const ast::BoolLiteral &node) {
    return llvm::ConstantInt::get(types.at("bool"), node.val, true);
}

llvm::Value *LLBuilder::operator()(const ast::FloatLiteral &node) {
    return llvm::ConstantFP::get(types.at("double"), llvm::APFloat(node.val));
}

llvm::Value *LLBuilder::operator()(const ast::StringLiteral &node) {
    return builder.CreateGlobalString(node.str);
}

void LLBuilder::operator()(const ast::EmptyDeclaration &) {
    // do nothing
    check(true); // why not
}

void LLBuilder::operator()(const ast::SimpleDeclar &node) {
    for (const auto &id: node.init_declars)
        codegen(*id);
}

void LLBuilder::operator()(const ast::FuncBody &node) {
    codegen(*node.comp_stmt);
}

void LLBuilder::operator()(const ast::TranslationUnit &node) {
    for (const auto &d: node.declars)
        codegen(*d);
    // finish global constructors function if it exists
    if (global_ctors_func) {
        builder.SetInsertPoint(&global_ctors_func->back());
        builder.CreateRetVoid();
    }
    delete_unused_declarations();
}

llvm::Value *LLBuilder::operator()(const ast::CallExpr &node) {
    check(node.func.value());
    llvm::Function *func = functions.at(node.func.value());
    called_functions.insert(func);

    vector<llvm::Value *> arg_vals;
    if (const ast::MemberAccessExpr *method_call = get_if<ast::MemberAccessExpr>(
            node.called_func.get())) {
        // get the 'this' object to argument list
        llvm::Value *object = codegen(*method_call->object);
        arg_vals.push_back(object);
    }
    if (node.ctor_call) {
        auto *id_expr = get_if<ast::IdExpr>(node.called_func.get());
        check(id_expr);
        llvm::Type *class_type = types.at(id_expr->id);
        llvm::AllocaInst *this_alloca = builder.CreateAlloca(class_type, nullptr, "ctor_this");
        arg_vals.push_back(this_alloca);
    }
    for (const ast::node_ptr<ast::Expr> &arg: node.args)
        arg_vals.push_back(codegen(*arg));

    llvm::Value *returned_val = builder.CreateCall(func, arg_vals);
    // calls should always return rvalue, that's why we load from
    // the ctor alloca
    return node.ctor_call ? create_load(arg_vals[0]) : returned_val;
}

llvm::Value *LLBuilder::operator()(const ast::SubscriptExpr &node) {
    llvm::Value *dest_val = codegen(*node.dest);
    check(dest_val->getType()->isPointerTy());
    llvm::Value *index_val = codegen(*node.index);
    return create_binary_op(dest_val, index_val, ast::Plus);
}

llvm::Value *LLBuilder::operator()(const ast::TernaryExpr &node) {
    string no = std::to_string(node.src_info.line_no);
    llvm::BasicBlock *then_bb = newBB("ter_then." + no),
                     *else_bb = newBB("ter_else." + no),
                     *end_bb = newBB("ter_end." + no);

    // generate cond
    llvm::Value *cond_val = codegen(*node.cond);
    builder.CreateCondBr(cond_val, then_bb, else_bb);
    // generate then
    builder.SetInsertPoint(then_bb);
    llvm::Value *then_val = codegen(*node.then);
    // basic block could've changed (if then contained && for example)
    then_bb = builder.GetInsertBlock();
    builder.CreateBr(end_bb);
    // generate else
    builder.SetInsertPoint(else_bb);
    llvm::Value *else_val = codegen(*node.else_);
    // basic block could've changed during else_val generation
    else_bb = builder.GetInsertBlock();
    builder.CreateBr(end_bb);
    // generate end
    builder.SetInsertPoint(end_bb);
    // if 'b' and 'c' were void expressions (probably calls to void function),
    // the value is discarded
    if (else_val->getType()->isVoidTy())
        return nullptr;
    llvm::PHINode *phi_node = builder.CreatePHI(then_val->getType(), 2);
    phi_node->addIncoming(then_val, then_bb);
    phi_node->addIncoming(else_val, else_bb);

    return phi_node;
}

void LLBuilder::class_first_pass(const ast::ClassDef &node) {
    string name = node.head->name;
    // this assumes no class forward declarations, namespaces or nested classes
    check(!types.contains(name), "class redefinition");

    llvm::StructType *class_type = llvm::StructType::create(context, name);
    // add the class to list of types before going in body in case of 'S *' member
    types[name] = class_type;

    vector<const ast::Decl *> fields;
    if (node.body)
        fields = class_first_pass(*node.body.value());
    vector<string> names;
    vector<llvm::Type *> field_types;
    for (const auto &m: fields) {
        names.push_back(m->id);
        field_types.push_back(get_llvm_type(m->type));
    }

    class_fields[class_type] = names;
    class_type->setBody(field_types);
}

std::vector<const ast::Decl *> LLBuilder::class_first_pass(const ast::MemberSpecification &node) {
    vector<const ast::Decl *> res;
    for (const auto &ms: node.list) {
        if (const auto *md = get_if<ast::MemberDeclaration>(ms.get())) {
            if (const auto *mdl = get_if<ast::MemberDeclaratorList>(md)) {
                for (const ast::node_ptr<ast::MemberDeclarator> &ast_decl: mdl->decls) {
                    // method declaration without definition
                    if (cpm::function_ty(ast_decl->type))
                        check(false, "method forward declaration");
                    // field
                    else
                        res.push_back(ast_decl.get());
                }
            } else if (const auto *fd = get_if<ast::FuncDef>(md)) {
                getFunction(*fd->declarator);
            }
        }
    }
    return res;
}

void LLBuilder::operator()(const ast::ClassDef &node) {
    class_first_pass(node);
    class_second_pass(node);
}

void LLBuilder::class_second_pass(const ast::ClassDef &node) {
    if (node.body)
        class_second_pass(*node.body.value());
}

void LLBuilder::class_second_pass(const ast::MemberSpecification &node) {
    for (const auto &ms: node.list)
        if (const auto *md = get_if<ast::MemberDeclaration>(ms.get()))
            if (const auto *fd = get_if<ast::FuncDef>(md))
                codegen(*fd);
}

llvm::Type *LLBuilder::get_llvm_type(cpm::Type *t) {
    if (cpm::SimpleType *st = cpm::simple_ty(t))
        return types.at(st->getTypeId());
    else if (cpm::PointerType *pt = cpm::pointer_ty(t)) {
        // special case of 'void*'
        if (st = cpm::simple_ty(pt->getElemType()); st && st->getTypeId() == "void")
            return getPtrToVoid();
        return llvm::PointerType::get(get_llvm_type(pt->getElemType()), 0);
    } else if (cpm::FunctionType *ft = cpm::function_ty(t)) {
        llvm::Type *ret_ty = get_llvm_type(ft->getRetType());
        std::vector<llvm::Type *> params;
        for (cpm::Type *p: ft->getParams())
            params.push_back(get_llvm_type(p));
        return llvm::FunctionType::get(ret_ty, params, ft->isVararg());
    } else if (cpm::ArrayType *at = cpm::array_ty(t)) {
        // represent arrays of unknown size by size 0 (if they were added to the lang)
        size_t size = at->getSize() ? at->getSize().value() : 0;
        return llvm::ArrayType::get(get_llvm_type(at->getElemType()),
                                    size);
    }
    compiler_error("get_llvm_type: unhandled case");
}

llvm::BasicBlock *LLBuilder::newBB(const string &name) {
    llvm::Function *func = getCurrentFunction();
    check(func);
    return llvm::BasicBlock::Create(context, name, func);
}

llvm::Value *LLBuilder::operator()(const ast::MemberAccessExpr &node) {
    llvm::Value *object = codegen(*node.object);
    // object is raw rvalue pointer in case of '->', or lvalue (llvm pointer) in case of '.'
    // -> we can treat like a pointer either way
    return getField(object, node.member);
}

llvm::Value *LLBuilder::getField(llvm::Value *object, const string &field) {
    auto *class_type = llvm::dyn_cast<llvm::StructType>(object->getType()->getPointerElementType());
    check(class_type);
    int idx = -1;
    const vector<string> &fields = class_fields.at(class_type);
    for (size_t i = 0; i < fields.size() && idx == -1; i++)
        if (field == fields[i])
            idx = i;
    check(idx != -1);

    string inst_name = string(class_type->getName()) + "." + field;
    // first index is 0 because we're already pointing to the object
    return builder.CreateGEP(class_type, object, {builder.getInt32(0), builder.getInt32(idx)},
                             inst_name);
}

llvm::Value *LLBuilder::convert(llvm::Value *val, llvm::Type *dest_ty) {
    llvm::Type *val_ty = val->getType();

    if (val_ty == dest_ty)
        return val;
    // to bool conversions
    else if (dest_ty == types.at("bool")) {
        llvm::Value *zero = llvm::Constant::getNullValue(val_ty);
        llvm::Value *res = create_binary_op(val, zero, ast::NotEqual);
        res->setName("tobool");
        return res;
    }
    // integers promotions
    else if (val_ty->isIntegerTy() && dest_ty->isIntegerTy()) {
        // bool widening, we want 'true' to be 1, not -MAX_INT
        if (val_ty == builder.getInt1Ty())
            return builder.CreateZExt(val, dest_ty, "int_conv");
        else
            return builder.CreateSExtOrTrunc(val, dest_ty, "int_conv");
    }
    // int to double
    else if (val_ty->isIntegerTy() && dest_ty->isDoubleTy())
        return builder.CreateSIToFP(val, dest_ty, "sitofp");
    // double to int
    else if (val_ty->isDoubleTy() && dest_ty->isIntegerTy())
        return builder.CreateFPToSI(val, dest_ty, "fptosi");
    // ptr to ptr
    else if (val_ty->isPointerTy() && dest_ty->isPointerTy())
        return builder.CreateBitCast(val, dest_ty, "ptrcast");
    compiler_error("unimplemented case in 'convert'");
}

LLBuilder::LLBuilder() :
        context(),
        module("basic", context),
        builder(context) {}

bool LLBuilder::verifyModule() const {
    return llvm::verifyModule(module, &llvm::errs());
}

llvm::Value *LLBuilder::operator()(const ast::LValToRValExpr &node) {
    llvm::Value *lvalue = codegen(*node.val);
    return create_load(lvalue);
}

void LLBuilder::dumpModule(std::ostream &os) {
    std::error_code ec;
    llvm::raw_os_ostream fs(os);
    module.print(fs, nullptr);
}

void LLBuilder::run(const ast::TranslationUnit *start_tu) {
    if (already_run)
        compiler_error("rerunning LLBuilder is not allowed, please use a new instance");
    already_run = true;
    codegen(*start_tu);
}

llvm::Value *LLBuilder::operator()(const ast::DefaultArgExpr &node) {
    return codegen(*node.expr);
}

llvm::Value *LLBuilder::operator()(const ast::ImplicitThisExpr &) {
    return get_this_object();
}

void LLBuilder::operator()(const ast::Stmt &node) {
    // don't generate ir for dead code
    if (builder.GetInsertBlock()->getTerminator())
        return;
    return std::visit(*this, node);
}

llvm::Value *LLBuilder::operator()(const ast::Expr &node) {
    return std::visit(*this, node);
}

void LLBuilder::operator()(const ast::Declaration &node) {
    return std::visit(*this, node);
}

void LLBuilder::operator()(const ast::ForInitStmt &node) {
    return std::visit(*this, node);
}

llvm::Value *LLBuilder::operator()(const ast::ArrToPtrExpr &node) {
    llvm::Value *val = codegen(*node.arr_expr);
    llvm::Type *val_ty = val->getType();
    // array should be lvalue, thus in llvm we're holding a pointer to array
    check(val_ty->isPointerTy());
    llvm::Type *array_ty = val_ty->getPointerElementType();
    check(array_ty->isArrayTy());

    return builder.CreateGEP(array_ty, val,
                             // first array, first element
                             {builder.getInt32(0), builder.getInt32(0)},
                             "arr_to_ptr");
}

llvm::Value *LLBuilder::operator()(const ast::CastExpr &node) {
    llvm::Value *val = codegen(*node.expr);
    return convert(val, node.type);
}

llvm::Value *LLBuilder::operator()(const ast::SizeofTypeExpr &node) {
    check(node.type);
    llvm::Type *type = get_llvm_type(node.type);
    // https://stackoverflow.com/questions/14608250/how-can-i-find-the-size-of-a-type
    llvm::Value *null = llvm::Constant::getNullValue(llvm::PointerType::get(type, 0));
    llvm::Value *null_plus_one = builder.CreateGEP(type, null, builder.getInt32(1));
    llvm::Value *sizeof_val = builder.CreatePtrToInt(null_plus_one, types.at("int"), "sizeof");
    return sizeof_val;
}

llvm::Value *LLBuilder::get_this_object() {
    if (this_rval)
        return this_rval;
    else if (this_lval) {
        this_rval = create_load(this_lval);
        return this_rval;
    }
    compiler_error("invalid use of 'this'");
}

void LLBuilder::initialize_global_var(llvm::Value *ptr, const ast::Expr &ast_init_val) {
    check(llvm::dyn_cast<llvm::GlobalVariable>(ptr));
    check(!builder.GetInsertBlock() && "global initialization while in a function");

    if (!global_ctors_func) {
        create_global_ctors_func();
    }

    // generate the value and store it
    builder.SetInsertPoint(&global_ctors_func->getBasicBlockList().back());
    llvm::Value *init_val = codegen(ast_init_val);
    builder.CreateStore(init_val, ptr);

    // reset insert point
    builder.ClearInsertionPoint();
}

void LLBuilder::create_global_ctors_func() {
    check(!global_ctors_func);
    global_ctors_func = llvm::Function::Create(
            llvm::FunctionType::get(builder.getVoidTy(), false),
            llvm::Function::InternalLinkage,
            // choose id with forbidden id char in c+-, '.' in this case
            "run_global_ctors.cpp",
            &module
    );
    global_ctors_func->setSection(".text.startup");
    llvm::BasicBlock::Create(context, "entry", global_ctors_func);

    llvm::StructType *struct_type = llvm::StructType::get(context,
                                                          {builder.getInt32Ty(),
                                                           global_ctors_func->getType(),
                                                           builder.getInt8PtrTy()});
    auto initializers = llvm::ConstantArray::get(
            llvm::ArrayType::get(
                    struct_type, 1
            ),
            llvm::ConstantStruct::get(struct_type,
                                      {
                                              // lowest priority
                                              builder.getInt32(65535),
                                              global_ctors_func,
                                              llvm::Constant::getNullValue(builder.getInt8PtrTy())
                                      }
            )
    );
    new llvm::GlobalVariable(
            module,
            initializers->getType(),
            true,
            llvm::GlobalVariable::AppendingLinkage,
            initializers,
            "llvm.global_ctors"
    );
}

llvm::Value *LLBuilder::convert(llvm::Value *val, cpm::Type *dest_ty) {
    return convert(val, get_llvm_type(dest_ty));
}

llvm::Value *LLBuilder::create_unary_minus(llvm::Value *val) {
    llvm::Value *zero = llvm::Constant::getNullValue(val->getType());
    return create_binary_op(zero, val, ast::BinaryOp::Minus);
}

llvm::Value *LLBuilder::create_load(llvm::Value *ptr) {
    llvm::Type *load_type = ptr->getType()->getPointerElementType();
    check(load_type);
    return builder.CreateLoad(load_type, ptr);
}

void LLBuilder::delete_unused_declarations() {
    vector<llvm::Function *> deleted_funcs;
    for (auto &f: module.functions())
        if (f.empty() && !called_functions.contains(&f))
            deleted_funcs.push_back(&f);
    for (auto *f: deleted_funcs)
        f->eraseFromParent();
}

std::string LLBuilder::get_function_name(const ast::FunctionDecl *func_decl) {
    string func_name = func_decl->id;
    if (!func_decl->params.empty()) {
        const auto &first_param = func_decl->params.at(0);
        if (first_param->declarator->id == "this") {
            cpm::PointerType *pt = cpm::pointer_ty(first_param->declarator->type);
            check(pt);
            cpm::SimpleType *st = cpm::simple_ty(pt->getElemType());
            check(st);
            return st->getTypeId() + "::"s + func_name;
        }
    }
    return func_name;
}

void LLBuilder::compiler_error(const std::string &msg) {
    throw std::runtime_error("LLBuilder: "s + (msg.empty() ? "unexpected internal error" : msg));
}

void LLBuilder::check(bool b, const std::string &msg) {
    if (!b)
        compiler_error(msg);
}