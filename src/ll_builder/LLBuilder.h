#pragma once

#include <variant>
#include <map>
#include <set>
#include <ostream>
#include <stdexcept>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/IR/CFG.h>


#include "ast/all_headers.h"
#include "type/DerivedTypes.h"

namespace cpm {
/**
 * This class generates the LLVM IR for given AST.
 *
 * It assumes that the AST has already been run through SemanticChecker.
 */
    class LLBuilder {

    public:

        explicit LLBuilder();

        /**
         * Run the llvm ir generation.
         *
         * UB if called more than once
         * @param ts
         */
        void run(const ast::TranslationUnit *start_tu);

        /**
        * Dump the module (llvm ir) to given stream.
        */
        void dumpModule(std::ostream &os);

        /**
         * Use the llvm verifier to verify correctness of the built module.
         * @return false on success, true on failure
         */
        bool verifyModule() const;

        /* expressions */
        llvm::Value *operator()(const ast::Expr &node);

        llvm::Value *operator()(const ast::IntLiteral &node);

        llvm::Value *operator()(const ast::IdExpr &node);

        llvm::Value *operator()(const ast::CharLiteral &node);

        llvm::Value *operator()(const ast::ThisExpr &node);

        llvm::Value *operator()(const ast::BoolLiteral &node);

        llvm::Value *operator()(const ast::FloatLiteral &node);

        llvm::Value *operator()(const ast::NullptrLiteral &node);

        llvm::Value *operator()(const ast::StringLiteral &node);

        llvm::Value *operator()(const ast::BinaryExpr &node);

        llvm::Value *operator()(const ast::LValToRValExpr &node);

        llvm::Value *operator()(const ast::AssignmentExpr &node);

        llvm::Value *operator()(const ast::CommaExpr &node);

        llvm::Value *operator()(const ast::CallExpr &node);

        llvm::Value *operator()(const ast::SubscriptExpr &node);

        llvm::Value *operator()(const ast::TernaryExpr &node);

        llvm::Value *operator()(const ast::PostIncrExpr &node);

        llvm::Value *operator()(const ast::UnaryExpr &node);

        llvm::Value *operator()(const ast::CastExpr &node);

        llvm::Value *operator()(const ast::SizeofTypeExpr &node);

        /**
         * Creates binary expression.
         *
         * Doesn't work for operators ast::LogicalAnd, ast::LogicalOr, these are
         * expected to be short-circuited.
         */
        llvm::Value *create_binary_op(llvm::Value *lhs, llvm::Value *rhs, ast::BinaryOp op);

        /**
         * Apply unary minus to a value.
         */
        llvm::Value *create_unary_minus(llvm::Value *val);

        /**
         * Handle access to class fields. Does not handle access to class methods.
         * @param node
         * @return
         */
        llvm::Value *operator()(const ast::MemberAccessExpr &node);

        llvm::Value *operator()(const ast::DefaultArgExpr &node);

        llvm::Value *operator()(const ast::ImplicitTypeCastExpr &node);

        llvm::Value *operator()(const ast::ImplicitThisExpr &node);

        llvm::Value *operator()(const ast::ArrToPtrExpr &node);

        llvm::Value *operator()(const ast::Condition &node);

        /* statements */
        void operator()(const ast::Stmt &node);

        void operator()(const ast::DeclarStmt &node);

        void operator()(const ast::ExprStmt &node);

        void operator()(const ast::BreakStmt &node);

        void operator()(const ast::ContinueStmt &node);

        void operator()(const ast::ReturnStmt &node);

        void operator()(const ast::CompoundStmt &node);

        void operator()(const ast::DoWhileStmt &node);

        void operator()(const ast::ForStmt &node);

        void operator()(const ast::IfStmt &node);

        void operator()(const ast::WhileStmt &node);

        /* declarations */
        void operator()(const ast::Declaration &node);

        void operator()(const ast::SimpleDeclar &node);

        /**
         *
         *
         * handles both normal functions and class methods
         * @param node
         * @return
         */
        void operator()(const ast::FuncDef &node);

        /**
         * Handles a class definition - creates the llvm struct, declares/defines all class
         * methods that are in the class body.
         *
         * ignores access specifiers (public, private) - they don't translate to anything in llvm
         * @param node
         * @return
         */
        void operator()(const ast::ClassDef &node);

        void operator()(const ast::EmptyDeclaration &node);

        /* for loop init statements */
        void operator()(const ast::ForInitStmt &node);

        void operator()(const ast::InitDeclarator &node);

        void operator()(const ast::FuncBody &node);

        void operator()(const ast::TranslationUnit &node);


    private:
        llvm::LLVMContext context;
        llvm::Module module;
        llvm::IRBuilder<> builder;
        // flag to avoid running a builder multiple times
        bool already_run = false;

        // map with <type name, llvm type>
        std::map<std::string, llvm::Type *> types = {
                {"int",       llvm::Type::getInt32Ty(context)},
                {"char",      llvm::Type::getInt8Ty(context)},
                {"bool",      llvm::Type::getInt1Ty(context)},
                {"double",    llvm::Type::getDoubleTy(context)},
                {"void",      llvm::Type::getVoidTy(context)},
                {"nullptr_t", getPtrToVoid()}
        };

        /** For each value, index it by the declarator based on which it was created. */
        std::map<const ast::Decl *, llvm::Value *> vals;

        /**
         * Every time a function is first declared, save it here.
         * This works with 'sco' within getFunction.
         */
        std::map<const ast::FunctionDecl *, llvm::Function *> functions;

        // syntactic sugar
        LLBuilder &codegen = *this;

        // basic blocks for 'break' jumps
        std::vector<llvm::BasicBlock *> break_bbs;
        // basic blocks for 'continue' jumps
        std::vector<llvm::BasicBlock *> continue_bbs;
        // basic block in current function from which we return
        llvm::BasicBlock *return_bb = nullptr;
        // here the return value is stored before jumping to return_bb
        llvm::AllocaInst *ret_val = nullptr;
        // if we're inside a class method, this value is a pointer to
        // the 'this' pointer
        llvm::Value *this_lval = nullptr;
        // if we're inside a class method and 'this' has already been used, this
        // points to the loaded pointer (rvalue)
        llvm::Value *this_rval = nullptr;
        // function that calls global constructors, is called before main
        llvm::Function *global_ctors_func = nullptr;

        // keep a list of functions that have been called in the program
        std::set<llvm::Function *> called_functions;

        /**
         * For each class, save it's fields in the order they were declared.
         *
         * This does not save class methods, only fields (variables).
         */
        std::map<llvm::StructType *, std::vector<std::string>> class_fields;

        /**
         * @return  the llvm function we're currently building in.
         *          nullptr if the insert point is not inside a function.
         */
        llvm::Function *getCurrentFunction();

        /**
         * Creates and returns a new BasicBlock which is put at the
         * end of current function.
         */
        llvm::BasicBlock *newBB(const std::string &name = "");

        /**
         * Returns the llvm function that corresponds to this declarator.
         * If the function doesn't exist yet, it is created.
         *
         * This works for normal functions and class methods.
         *
         * @param node
         * @return
         */
        llvm::Function *getFunction(const ast::FunctionDecl &node);

        /**
         * Get the internal representation of 'void *'. This cannot simply be something like
         * 'llvm::PointerType::get(llvm::VoidTy)' because this type would not pass llvm verifier.
         * @return
         */
        llvm::PointerType *getPtrToVoid() {
            return builder.getInt8PtrTy();
        }

        /**
         * Create a shortcircuit for '&&' or '||'.
         * @param node
         * @return
         */
        llvm::Value *create_shortcircuit(const ast::BinaryExpr &node);

        /**
         * For given cpm::Type, return corresponding llvm::Type.
         *
         * @param type
         * @return
         */
        llvm::Type *get_llvm_type(cpm::Type *type);

        /**
         * Creates instruction to convert value to dest type.
         */
        llvm::Value *convert(llvm::Value *val, llvm::Type *dest_ty);

        /**
         * Creates instruction to convert value to llvm equivalent of dest type.
         */
        llvm::Value *convert(llvm::Value *val, cpm::Type *dest_ty);

        /**
         * Access a class field.
         *
         * This method is separated because this is needed both in MemberAccessExpr and
         * in IdExpr.
         * @param objectPtr
         * @param field
         * @return
         */
        llvm::Value *getField(llvm::Value *objectPtr, const std::string &field);

        /**
         * Creates the increment or decrement (by one) operation on a value.
         * This does the shared work for pre/post increment/decrement.
         *
         * @param lval  value to be increment, should be lvalue (e.g. can be loaded from)
         * @param incr  true if increment, false if decrement
         * @return      the result rvalue of the increment/decrement
         */
        llvm::Value *incr_decr(llvm::Value *lval, bool incr);

        /**
         * This:
         * 1) Creates the llvm struct for this class
         * 2) Adds all the class fields into the body, and saves their order in 'class_fields' for futures GEPs
         * 3) Declares the class methods without defining them
         * @param node
         */
        void class_first_pass(const ast::ClassDef &node);

        /**
         * Get the declarations of all fields in the class, and declare all methods in class.
         * @param node
         * @return list of class fields (does not contain class methods)
         */
        std::vector<const ast::Decl *> class_first_pass(const ast::MemberSpecification &node);

        /**
         * Runs the second pass on a class. That is, it:
         * 1) does codegen for bodies of defined methods
         * @param node
         */
        void class_second_pass(const ast::ClassDef &node);

        /**
         * ignores fields, defines methods
         * @param node
         */
        void class_second_pass(const ast::MemberSpecification &node);

        /**
         * Return the rvalue with 'this' pointer/
         * @return
         */
        llvm::Value *get_this_object();

        /**
         * Initialize global variable with given value.
         * @param ptr
         * @param ast_init_val
         */
        void initialize_global_var(llvm::Value *ptr, const ast::Expr &ast_init_val);

        void create_global_ctors_func();

        /**
         * Create a load from a value.
         *
         * Expects the pointer to be typed, won't work with opaque pointers.
         */
        llvm::Value *create_load(llvm::Value *ptr);

        /**
         * Delete functions that have been declared but not defined, and
         * that have not been used in the program. e.g. useless declarations
         */
        void delete_unused_declarations();

        std::string get_function_name(const ast::FunctionDecl *func_decl);

        /**
         * Report unexpected compiler error.
         *
         * Throws std::runtime_error.
         */
        [[noreturn]]
        void compiler_error(const std::string &msg);

        /**
         * Check if b is true, call compiler_error otherwise.
         *
         * Replacement for 'assert'
         */
        void check(bool b, const std::string &msg = "");
    };

}