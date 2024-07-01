#pragma once

#include <string>
#include <vector>
#include <set>
#include <tuple>

#include "utils/Context.h"
#include "semantic_checker/scope/Scope.h"
#include "semantic_checker/scope/Class.h"
#include "type/TypeManager.h"
#include "ast/all_headers.h"
#include "type/Type.h"
#include "type/DerivedTypes.h"


namespace cpm::sc {
/**
 * Perform semantic analysis on an AST.
 *
 *
 * */
    class SemanticChecker {
    public:
        SemanticChecker(cpm::Context &context,
                        std::ostream &warning_os) :
                context(context),
                warning_os(warning_os) {}

        /**
         * Run the checker on AST.
         *
         * Should only be called once, UB if it is called multiple times.
         */
        void run(ast::TranslationUnit &node);

        enum ValueType {
            LValue,
            RValue
        };

        /**
         * Represents a value that's the result of an expression.
         *
         */
        struct Value {
            cpm::Type *type;
            ValueType valtype;

            [[nodiscard]] std::string str() const {
                std::string val = valtype == LValue ? "lvalue" : "rvalue";
                std::string ty = cpm::to_string(type);
                return val + " of type " + ty;
            }
        };

        /**
         * all 'operator ()' methods on ast nodes are public so that we can use 'std::visit'
         */

        /* expressions */
        /**
         * Visit an expression, and return its type and value category.
         *
         * Takes in reference to node_ptr so that the original node_ptr (probably member
         * of an Node) can be changed in place.
         * @param node_ptr
         * @param arr_to_ptr whether automatic array to pointer decay should take place if
         *                   type of the expression is array
         * @return
         */
        Value operator()(ast::node_ptr<ast::Expr> &node_ptr, bool arr_to_ptr = true);

        Value operator()(ast::IntLiteral &);

        Value operator()(ast::IdExpr &);

        Value operator()(ast::CharLiteral &);

        /* should this get info about current method class? */
        Value operator()(ast::ThisExpr &);

        Value operator()(ast::BoolLiteral &);

        Value operator()(ast::FloatLiteral &);

        Value operator()(ast::NullptrLiteral &);

        Value operator()(ast::StringLiteral &);

        Value operator()(ast::SizeofTypeExpr &);

        Value operator()(ast::BinaryExpr &);

        Value operator()(ast::AssignmentExpr &);

        Value operator()(ast::CommaExpr &);

        Value operator()(ast::CallExpr &);

        Value operator()(ast::SubscriptExpr &);

        Value operator()(ast::TernaryExpr &);

        Value operator()(ast::PostIncrExpr &);

        Value operator()(ast::UnaryExpr &);

        Value operator()(ast::CastExpr &);

        /**
         * This handles member access to class variables; not method calls through object.
         * @param node
         * @return
         */
        Value operator()(ast::MemberAccessExpr &);

        Value operator()(ast::DefaultArgExpr &);

        Value operator()(ast::ImplicitTypeCastExpr &);

        Value operator()(ast::LValToRValExpr &);

        Value operator()(ast::ImplicitThisExpr &);

        Value operator()(ast::ArrToPtrExpr &);

        /* statements */
        void operator()(ast::Stmt &);

        void operator()(ast::DeclarStmt &);

        void operator()(ast::ExprStmt &);

        void operator()(ast::BreakStmt &);

        void operator()(ast::ContinueStmt &);

        void operator()(ast::ReturnStmt &);

        void operator()(ast::CompoundStmt &);

        void operator()(ast::DoWhileStmt &);

        void operator()(ast::ForStmt &);

        void operator()(ast::IfStmt &);

        void operator()(ast::WhileStmt &);

        /* declarations */
        void operator()(ast::Declaration &);

        void operator()(ast::SimpleDeclar &);

        /**
         * check that the declarator type is valid
         * @param node
         */
        void operator()(ast::Decl &);

        /**
         * check that the function declarator is valid
         * @param node
         */
        void operator()(ast::FunctionDecl &);

        void operator()(ast::FuncDef &);

        void operator()(ast::EmptyDeclaration &);

        void operator()(ast::ClassDef &);

        /**
         * These methods do the first pass of the class.
         * During the first pass, non-static members (both
         * fields and methods) are added to the scope of the class,
         * first only fields, and then methods.
         *
         * What is skipped is the bodies and default arguments of methods.
         *
         */
        void class_first_pass(ast::ClassDef &);

        void class_first_pass(ast::MemberSpecification &);

        /**
         * If the declaration is a function definition, don't process it and return pointer
         * to it, along with the access, to process it later. Otherwise, return nullptr.
         */
        std::pair<ast::FuncDef *, ast::AccessModifier> class_first_pass(ast::MemberDeclaration &);

        void class_first_pass(ast::MemberDeclarator &);

        void class_first_pass(ast::FuncDef &);

        void class_second_pass(ast::ClassDef &);

        void class_second_pass(ast::MemberSpecification &);

        void class_second_pass(ast::MemberDeclaration &);

        void class_second_pass(ast::FuncDef &);

        /* for loop init statements */
        void operator()(ast::ForInitStmt &);

        /**
         *
         * @param node
         * @param current_type the type that was declared by previous DeclSpecSeq
         */
        void operator()(ast::InitDeclarator &);

        void operator()(ast::FuncBody &);

        /**
         * The condition expression must be implicitly convertible to bool.
         * @param node
         */
        void operator()(ast::Condition &);

        void operator()(ast::TranslationUnit &);

    private:

        /**
         * Processes 'val' and tries to convert it to rvalue of given type.
         * @param val node to be converted
         * @param dest destination type
         * @param node used for error reporting
         * @param throw_error   if true, this calls 'error()' if the conversion is not possible;
         *                      if false, nullptr is returned to signal failure
         * @return   new expression node that contains necessary conversions on success;
         *           nullptr or 'error()' if conversion is not possible
         */
        ast::node_ptr<ast::Expr> convert_to_rval(ast::node_ptr<ast::Expr> val, cpm::Type *dest,
                                                 const ast::Node &node, bool throw_error = true);

        /**
         * Tries to convert value to rvalue of given type, assuming that the value has provided type and value type.
         * @param val value to be converted
         * @param from_ty type of the value
         * @param from_vt value type of the value
         * @param dest destination type
         * @param node used for error reporting
         * @param throw_error   if true, this calls 'error()' if the conversion is not possible;
         *                      if false, nullptr is returned to signal failure
         * @return   new expression node that contains necessary conversions on success;
         *           nullptr or 'error()' if conversion is not possible
         */
        ast::node_ptr<ast::Expr>
        convert_to_rval(ast::node_ptr<ast::Expr> val, Value from, cpm::Type *dest,
                        const ast::Node &node, bool throw_error = true);

        /**
         * Helper function for 'convert' error reporting.
         */
        ast::node_ptr<ast::Expr>
        convert_error(Value from, cpm::Type *dest, const ast::Node &node, bool throw_error);

        /**
         * Check if function declarator is correct:
         * - the function type is valid
         * - parameter names are not repeated
         * - default arguments don't contain gaps
         * - optionally if default arg values can be (and should be) converted to the parameter type
         *
         * Throws error() on failure.
         * @param node
         * @param process_def_args
         */
        void operator()(ast::FunctionDecl &node, bool process_def_args);

        static bool is_numerical(cpm::SimpleType *simple_ty);

        static bool is_numerical(cpm::Type *type);

        static bool is_integral(cpm::Type *type);

        static bool is_integral(cpm::SimpleType *simple_ty);

        static bool is_floating(Type *type);

        static bool is_floating(SimpleType *simple_ty);

        /**
         * Checks if a type is const-qualified.
         *
         * True can only be returned for SimpleType and PointerType instances.
         * @param type
         * @return
         */
        static bool is_const(cpm::Type *type);

        static bool is_void(cpm::Type *type);

        static bool is_void(cpm::SimpleType *simple_ty);

        static bool is_int(cpm::SimpleType *simple_ty) {
            return simple_ty && simple_ty->getTypeId() == "int";
        }

        static bool is_int(cpm::Type *type) {
            return is_int(simple_ty(type));
        }

        static bool is_double(cpm::SimpleType *simple_ty) {
            return simple_ty && simple_ty->getTypeId() == "double";
        }

        static bool is_bool(cpm::SimpleType *simple_ty) {
            return simple_ty && simple_ty->getTypeId() == "bool";
        }

        static bool is_char(cpm::SimpleType *simple_ty) {
            return simple_ty && simple_ty->getTypeId() == "char";
        }

        static bool is_nullptr(cpm::SimpleType *simple_ty) {
            return simple_ty && simple_ty->getTypeId() == "nullptr_t";
        }

        static bool is_nullptr(cpm::Type *type) {
            return is_nullptr(cpm::simple_ty(type));
        }

        /**
         * Checks whether a SimpleType with given name exists and is accessible to the user.
         */
        bool type_exists(const std::string &type_id);

        /**
         * Throws an 'std::runtime_error' exception with the error.
         * @param msg
         * @param node
         */
        [[noreturn]] void error(const std::string &msg, const ast::Node &node);

        /**
         * Throws an 'std::runtime_error' exception with the error.
         * Used to report unimplemented functionality that the user might expect to work.
         * Otherwise is the same as error().
         * @param msg
         * @param node
         */
        [[noreturn]] void compiler_error(const std::string &msg, const ast::Node &node);

        /* Reports warnings about the source code. */
        void warning(const std::string &msg, const ast::Node &node);

        /* factories for types */
        cpm::SimpleType *getSimpleType(const std::string &type_id, bool is_const) {
            return context.getSimpleType(type_id, is_const);
        }

        cpm::PointerType *getPointerType(cpm::Type *elem_type, bool is_const) {
            return context.getPointerType(elem_type, is_const);
        }

        cpm::ArrayType *getArrayType(cpm::Type *elem_type, std::optional<size_t> size) {
            return context.getArrayType(elem_type, size);
        }

        cpm::FunctionType *getFunctionType(cpm::Type *ret_ty, std::vector<cpm::Type *> params,
                                           bool is_vararg) {
            return context.getFunctionType(ret_ty, std::move(params), is_vararg);
        }

        /* shortcuts for frequently used types */
        cpm::SimpleType *getIntType(bool is_const = false);

        cpm::SimpleType *getBoolType(bool is_const = false);

        cpm::SimpleType *getVoidType();

        cpm::SimpleType *getDoubleType(bool is_const = false);

        cpm::SimpleType *getCharType(bool is_const = false);

        cpm::SimpleType *getNullptrType();

        /**
         * Tries to find a common type for two types.
         * There are two options:
         * - if they are two simple types, converting one type to the another (e.g. for 'int' and 'double' return 'double')
         * - if they are two similar pointer types, finding a pointer type that matches const requirements of both
         *     (e.g. for 'const int * *' and 'int * const *', return 'const int * const *')
         * @param t1
         * @param t2
         * @return  common type for t1 and t2 if it exists,
         *          nullptr if there's no common type
         */
        cpm::Type *determine_common_type(cpm::Type *t1, cpm::Type *t2);

        /**
         * Get the common type for two simple types.
         * Returns const-unqualified common type.
         * @param t1
         * @param t2
         * @return
         */
        cpm::SimpleType *common_type(cpm::SimpleType *t1, cpm::SimpleType *t2);

        /**
         * Get the common type for two pointers types.
         * Disregards const qualifiers at the first level, e.g. p1->isConst() and p2->isConst();
         * does consider const qualifiers deeper, from the second level down.
         * @param p1
         * @param p2
         * @return
         */
        cpm::PointerType *common_type(cpm::PointerType *p1, cpm::PointerType *p2);

        /**
         * Assuming that two types are the same stripped of const qualifiers, return
         * type that is at least as const-strong as both of them.
         *
         * e.g. for 'const int * *' and 'int * const *' return 'const int * const *'
         * @param t1
         * @param t2
         * @return
         */
        cpm::Type *common_type_rec(cpm::Type *t1, cpm::Type *t2);

        /**
         * For operands of type 'lhs' and 'rhs' and binary operator 'op', return
         * the types to which 'lhs' and 'rhs' have to be converted to make this a
         * viable operation, and the result type of the operation.
         *
         * If it's impossible, return {nullptr, nullptr, nullptr}.
         * @param lhs
         * @param rhs
         * @param op
         * @return
         */
        std::tuple<cpm::Type *, cpm::Type *, cpm::Type *>
        conversions_for_bin_op(cpm::Type *lhs, cpm::Type *rhs, ast::BinaryOp op);

        /**
         * Returns true if expression of type 't' is viable for the '++' or '--' operator
         * (in either the preincrement or the postincrement form, doesn't matter).
         */
        bool viable_incr_type(cpm::Type *t);

        /**
         * Gets the class type of the object that is being accessed
         * either directly or indirectly threw pointer.
         *
         * Throws errors when the resulting type is not a class type.
         * @param node
         * @return
         */
        cpm::SimpleType *
        get_underlying_type(Value accessed_object, bool ptr_access, const ast::Node &node);

        /**
         * Return type that has the first constable subtype (pointer or simple) consted.
         *
         * This is used for const instances of a class.
         *
         * Errors for functions.
         * @param type
         * @return
         */
        cpm::Type *const_type(cpm::Type *type);


        /**
         * Adds a new scope, sets it to current scope and makes
         * the up-until-now scope parent of this scope.
         * @param scope nullptr if new scope should be created,
         *              otherwise the new scope
         * @return the new scope
         */
        Scope *addScope(Scope *scope = nullptr);

        /**
         * Sets 'current_scope' to the parent of 'current_scope'.
         */
        void dropScope();

        /**
         * Declare this function in current scope, or possibly update it if it has already been declared.
         *
         * If class is currently being defined (defined_class != ""), it will automatically add the implicit
         * 'this' to start of parameter list.
         *
         * The updating relates to default argument values - if they were not defined before, they may
         * be now.
         * @param func_decl
         * @param paq
         * @param node
         * @return ast::Decl that corresponds to the first declaration of this function
         */
        const ast::FunctionDecl *
        declareFunction(ast::FunctionDecl *func_decl, bool process_def_args = true);

        /**
         * Adds function of given type to current scope, if it doesn't already exist there.
         *
         * Reports an error when the name is a different kind of symbol, or when the function type
         * differs from a previous declaration only in return type.
         * @param id
         * @param func_type
         * @param node
         * @return ast::Decl that corresponds to the first declaration of this function
         *         (if this is the first declaration of this function, addr of 'node' argument will be returned)
         */
        const ast::FunctionDecl *addFunctionToScope(const ast::FunctionDecl *func_decl);

        enum FuncCMP {
            // param types match c-unqualified and return matches
            MATCH,
            // param types match c-unqualified, but return type differs
            RET_DIFF,
            // at least one param type doesn't match c-unqualified
            NO_MATCH
        };

        FuncCMP cmpFuncSignatures(cpm::FunctionType *t1, cpm::FunctionType *t2);

        /**
         * Tries to select function from current scope (or its ancestors) that matches arg_types.
         *
         * If the call is ambiguous, calls error().
         * @param scope
         * @param arg_types
         * @param node  used for error reporting
         * @return
         */
        ScopeValue resolveCallOverload(const std::string &func_name, Scope *scope,
                                       const std::vector<cpm::Type *> &arg_types,
                                       const ast::CallExpr &node);

        /**
         * How can a type be implicitly converted into another type?
         */
        // important: order matters, the values are compared by '<'
        enum TypeMatch {
            // the types match c-unqualified, e.g. "const int" and "int"
            EXACT = 0,
            // the destination type element type has extra const qualification
            // compared to argument type, e.g. "const int*" and "int*"
            CONST = 1,
            // start type requires type conversion to become destination type,
            // "double" and "int"
            CONVERSION = 2,
            // there's no way to implicitly convert start type to destination type,
            // e.g. "int" and "int *"
            NONE = 3
        };

        /**
         * Determines how (if) an rvalue of type 'from' can be converted to
         * an rvalue of type 'dest'.
         */
        TypeMatch implicitly_convertible(cpm::Type *start, cpm::Type *dest);

        /**
         * Checks if rvalue of type 'start_ty' can be converted to rvalue of type
         * 'dest_ty'.
         */
        bool explicitly_convertible(cpm::Type *start_ty, cpm::Type *dest_ty);

        /**
         * Checks if a list of type matches is at least as good as another one.
         */
        static bool
        better_or_same_match(const std::vector<TypeMatch> &m1, const std::vector<TypeMatch> &m2);

        /**
         * Check if 'scope' is the current scope or an ancestor of the current scope.
         * @param scope
         * @return
         */
        bool inside_scope(Scope *scope);

        /**
         * Return t without const qualifier (if there is one).
         * @param t
         * @return
         */
        cpm::Type *const_unqualified_type(cpm::Type *t);

        /**
         * Check if t1 and t2 are c-unqualified the same,
         * except that t1->elem_type is c-qualified and t2->elem_type is not.
         *
         * e.g. "const int[5]" and "int[5]" return true,
         *      "int * const *" and "int ** const" return true
         */
        bool const_stronger_elem_type(cpm::Type *t1, cpm::Type *t2);

        /**
         * Check if name look up of id matches non-static class member.
         * @param id
         * @return
         */
        bool refers_to_class_member(const std::string &id);

        /**
         * Check if the type is incomplete.
         * @param type
         * @return
         */
        bool incomplete_type(cpm::Type *type);

        /**
         * Get the type that is the result of 'sizeof' expressions.
         * @return
         */
        cpm::Type *getSizeofType();

        /**
         * Add the implicit 'this' parameter to ast function declarator
         * @param func_decl
         */
        void add_this_to_func_decl(ast::FunctionDecl *func_decl);

        /**
         * Check if the type is valid (doesn't break a rule specified in 'BP text').
         *
         * Throws error() with description if the type is not valid.
         */
        void validate_type(cpm::Type *type, const ast::Node &node);

        /**
         * Check if two types are similar (same when stripped of const qualifiers);
         * @param t1
         * @param t2
         * @return
         */
        bool similar_types(Type *t1, Type *t2);

        /**
         * Add forward declarations of the following functions to the start
         * of a translation unit:
         *      printf, scanf, malloc, free, sprintf, sscanf
         *
         * @param tu
         */
        void add_builtin_functions(ast::TranslationUnit &tu);

        /**
         * Create ast node that corresponds to forward declaration of a function.
         */
        ast::node_ptr<ast::Declaration> create_func_declaration_node(
                const std::string &func_name, Type *ret_type,
                std::vector<ast::node_ptr<ast::Param>> params,
                bool is_vararg
        );

        /**
         * Create an ast::Param node.
         */
        ast::node_ptr<ast::Param> make_param(Type *type, const std::string &name,
                                             std::optional<ast::node_ptr<ast::Expr>> def_val = std::nullopt,
                                             const ast::SourceInfo &src_info = ast::SourceInfo());

        /**
         * Get the function name, adds "XXX::" for class methods to the decl id.
         */
        std::string get_function_name(const ast::FunctionDecl *func_decl);

        /**
         * Check if type is a pointer to a complete type, and if so, return
         * the PointerType. Otherwise return nullptr.
         */
        cpm::PointerType *pointer_to_complete_type(cpm::Type *type);

        //---------------- FIELDS ---------------------

        // syntactic sugar to simplify calling of the '()' operator
        SemanticChecker &process = *this;

        cpm::Context &context;
        std::ostream &warning_os;

        /**
        * This flag indicates whether the next CompoundStmt should
        * add a new scope or not.
        *
        * The exceptions, when this is false, are:
        *  - function body
        *  - for statement
        */
        bool cs_new_scope = true;

        // non class scopes
        std::vector<std::unique_ptr<Scope>> scopes;
        // user defined classes (scopes)
        std::map<std::string, std::unique_ptr<Class>> classes;
        Scope *global_scope = nullptr;
        Scope *current_scope = nullptr;
        // return type of the function we're currently in, or nullptr if we're not inside a function
        cpm::Type *curr_ret_type = nullptr;
        // how many loops deep are we in?
        size_t loop_level = 0;

        /**
         * For each declared function (including class methods and constructors),
         * this holds the list of default argument values. If given argument doesn't have a
         * default value, it is represented by nullptr in the list.
         *
         * If a function has multiple declarations, declarator of first one is used for the map key.
         */
        using DefArgList = std::vector<ast::Expr *>;
        std::map<const ast::Decl *, DefArgList> funcs_def_args;
        /**
         * For each default value, save what was the Value result of visiting the expression.
         * This prevents revisiting the expression when 'process' is called on the ast::DefaultArgExpr
         */
        std::map<const ast::Expr *, Value> def_arg_vals;


        /**
         * For each function (identified by the declarator of it's first declaration),
         * save here whether it has already been defined or not.
         * This is used to prevent function redefinitions.
         */
        std::set<const ast::Decl *> defined_funcs;

        /**
         * class that's being currently defined, "" for no class
         */
        std::string defined_class;
        ast::AccessModifier current_access;
        // store valid types that have already been checked
        std::set<cpm::Type *> valid_types_cache;
        // contain const-unqualified versions of incomplete types
        std::set<cpm::Type *> incomplete_types = {
                getVoidType()
        };
    };

}
