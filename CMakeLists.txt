cmake_minimum_required(VERSION 3.21)
project(bakalarka_jazyk)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -Wunused-variable -pedantic)

find_package(antlr4-generator 4.13 REQUIRED)
find_package(antlr4-runtime 4.13 REQUIRED)

# boost
find_package(Boost 1.82 REQUIRED COMPONENTS program_options)
add_library(Boost_program_options STATIC IMPORTED)
include_directories(${Boost_INCLUDE_DIRS})

if(DEFINED ENV{ANTLR4_JAR_LOCATION})
    SET(ANTLR4_JAR_LOCATION $ENV{ANTLR4_JAR_LOCATION})
    MESSAGE("ANTLR4_JAR_LOCATION ${ANTLR4_JAR_LOCATION}")
endif()

if(APPLE)   # from antlr4 PJP demo
    add_definitions(-DNO_SOURCE_LOCATION)
endif()

antlr4_generate(ParserAntlr
        ${CMAKE_CURRENT_SOURCE_DIR}/src/parser/grammar/CPM.g4
        BOTH # if I put BOTH here, the cmake fails
        FALSE # listener
        FALSE # visitor
        )

set_source_files_properties(
        ${ANTLR4_SRC_FILES_ParserAntlr}
        PROPERTIES
            GENERATED TRUE
            COMPILE_OPTIONS "-w")

find_package(LLVM REQUIRED CONFIG 14...<15)

# SYSTEM is important to ignore compiler warnings produced by the llvm libs,
# https://stackoverflow.com/questions/1867065/how-to-suppress-gcc-warnings-from-library-headers
# -- the pragma push solution didn't work for me the first time I tried
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

include_directories(src) # for simpler including, such as #include "ast/..."

add_library(ast STATIC
        src/ast/expr/literals/IntLiteral.cpp
        src/ast/expr/AssignmentExpr.cpp
        src/ast/decl/declaration.cpp
        src/ast/decl/Declarator.cpp
        src/ast/expr/IdExpr.cpp
        src/ast/decl/InitDeclarator.cpp
        src/ast/decl/SimpleDeclaration.cpp
        src/ast/TranslationUnit.cpp
        src/ast/expr/literals/CharLiteral.cpp
        src/ast/expr/operators.cpp
        src/ast/func/Param.cpp
        src/ast/stmt/stmt.cpp
        src/ast/stmt/CompoundStmt.cpp
        src/ast/stmt/DeclarStmt.cpp
        src/ast/stmt/ExprStmt.cpp
        src/ast/func/FuncBody.cpp
        src/ast/func/FuncDef.cpp
        src/ast/expr/CommaExpr.cpp
        src/ast/expr/BinaryExpr.cpp
        src/ast/stmt/ReturnStmt.cpp
        src/ast/stmt/BreakStmt.cpp
        src/ast/stmt/IfStmt.cpp
        src/ast/stmt/WhileStmt.cpp
        src/ast/stmt/DoWhileStmt.cpp
        src/ast/stmt/forInitStmt.cpp
        src/ast/stmt/ForStmt.cpp
        src/ast/stmt/Condition.cpp
        src/ast/expr/SubscriptExpr.cpp
        src/ast/expr/CallExpr.cpp
        src/ast/expr/ThisExpr.cpp
        src/ast/expr/TernaryExpr.cpp
        src/ast/decl/EmptyDeclaration.cpp
        src/ast/base/SourceInfo.cpp
        src/ast/base/Node.cpp
        src/ast/expr/PostIncrExpr.cpp
        src/ast/expr/literals/BoolLiteral.cpp
        src/ast/expr/literals/StringLiteral.cpp
        src/ast/expr/literals/NullptrLiteral.cpp
        src/ast/expr/literals/FloatLiteral.cpp
        src/ast/expr/SizeofTypeExpr.cpp
        src/ast/expr/UnaryExpr.cpp
        src/ast/expr/CastExpr.cpp
        src/ast/class/MemberDeclarator.cpp
        src/ast/class/MemberDeclaratorList.cpp
        src/ast/class/MemberDeclaration.cpp
        src/ast/class/MemberSpecification.cpp
        src/ast/class/ClassHead.cpp
        src/ast/class/ClassDef.cpp
        src/ast/class/MemberAccessExpr.cpp
        src/ast/expr/DefaultArgExpr.cpp
        src/ast/class/AccessModifier.h
        src/ast/expr/ImplicitTypeCastExpr.cpp
        src/ast/expr/LValToRValExpr.cpp
        src/ast/expr/ImplicitThisExpr.cpp
        src/ast/expr/ArrToPtrExpr.cpp
        src/ast/decl/Decl.cpp
        src/ast/decl/FunctionDecl.cpp)

add_library(types STATIC
        src/type/TypeManager.cpp
        src/type/Type.cpp
        src/type/DerivedTypes.cpp)

add_library(utils STATIC
        src/utils/CompilationError.cpp src/utils/Context.cpp src/utils/Context.h)

add_library(sc STATIC
        src/semantic_checker/SemanticChecker.cpp
        src/semantic_checker/scope/Scope.cpp
        src/semantic_checker/scope/Class.cpp
        src/semantic_checker/scope/ScopeValue.cpp
        )
target_link_libraries(sc PUBLIC types ast)

add_library(llbuilder STATIC
        src/ll_builder/LLBuilder.cpp
        )
llvm_config(llbuilder USE_SHARED support core irreader dump)

add_library(astdump STATIC
        src/ast_dumper/AstDumper.cpp
        )

add_library(parser STATIC
        src/parser/Parser.cpp
        src/parser/ParseTreeVisitor.cpp
        ${ANTLR4_SRC_FILES_ParserAntlr}
        )
target_include_directories(parser PUBLIC ${ANTLR4_INCLUDE_DIR_ParserAntlr})
target_include_directories(parser SYSTEM PUBLIC ${ANTLR4_INCLUDE_DIR})
target_link_libraries(parser PUBLIC antlr4_static types ast)

add_executable(cpm src/main.cpp)
target_link_libraries(cpm PRIVATE ast types utils sc llbuilder astdump parser)
target_link_libraries(cpm PUBLIC ${Boost_LIBRARIES})

include(CTest)
if(BUILD_TESTING)
    if(NOT CLANG_EXECUTABLE)
        # we need clang to compile ir to executable
        find_program(CLANG_EXECUTABLE clang REQUIRED)
    endif()

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tests/configure.cmake.h.in
            ${CMAKE_CURRENT_BINARY_DIR}/tests/configure.cmake.h)
    include_directories(${CMAKE_CURRENT_BINARY_DIR})

    function(create_tests_from_files)
        cmake_parse_arguments(PARSER "" "NAME;FILE;GLOB" "LIBS" ${ARGN})

        if(NOT PARSER_NAME)
            message(FATAL_ERROR "NAME argument missing")
        endif()
        if(NOT PARSER_FILE)
            message(FATAL_ERROR "FILE argument missing")
        endif()

        add_executable(test-${PARSER_NAME} ${PARSER_FILE})
        target_link_libraries(test-${PARSER_NAME} PUBLIC ${PARSER_LIBS})

        file(GLOB TESTFILES LIST_DIRECTORIES false CONFIGURE_DEPENDS ${PARSER_GLOB})
        foreach(FILENAME IN LISTS TESTFILES)
            get_filename_component(BASE "${FILENAME}" NAME)
            add_test(NAME "[${PARSER_NAME}]${BASE}" COMMAND test-${PARSER_NAME} ${FILENAME})
        endforeach()
    endfunction()

    create_tests_from_files(NAME parsing-invalid FILE tests/parsing-invalid.cpp GLOB "${CMAKE_CURRENT_SOURCE_DIR}/tests/invalid_inputs/parsing/*.cpp" LIBS utils parser)
    create_tests_from_files(NAME sc-invalid FILE tests/sc-invalid.cpp GLOB "${CMAKE_CURRENT_SOURCE_DIR}/tests/invalid_inputs/sema/*.cpp" LIBS utils parser sc)
    create_tests_from_files(NAME astdump FILE tests/astdump.cpp GLOB "${CMAKE_CURRENT_SOURCE_DIR}/tests/valid_inputs/*.cpp" LIBS utils parser sc astdump)
    create_tests_from_files(NAME run FILE tests/run.cpp GLOB "${CMAKE_CURRENT_SOURCE_DIR}/tests/valid_inputs/*.cpp" LIBS utils parser sc llbuilder)
    llvm_config(test-run USE_SHARED support core irreader dump)
endif()
