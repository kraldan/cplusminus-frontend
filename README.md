# C+- compiler frontend 

This repository contains a frontend for the C+- compiler frontend. 
The frontend is implemented in C++, using ANTLR4 for parsing and 
LLVM IR for intermediate representation. The compiler takes in 
a C+- source code and emits (unoptimized) LLVM IR. 

# C+- language
C+- is an (almost) subset of C++. 
In a nutshell, it's C with classes. More in depth feature 
specification can be found in the thesis/ directory. 

We say C+- is an 'almost' subset of C++ because it breaks from C++ 
in a few places, including:
* break and continue can branch from multiple nested loops, 
* class definition doesn't have to be followed by a semicolon.

# BI-PJP students
FIT CTU students taking the BI-PJP source might find this project 
helpful to help them with their LLVM IR generation. 
For that, please go look into to the *src/ll_builder/* directory. 
Note that since this project is built with LLVM 14, some things 
will very likely not work if you use a newer LLVM version. For 
example, we use typed pointers and getPointerElementType method, 
but this is not supported since LLVM 17. 

# Installation
You'll need:
* LLVM 14 libraries
* ANTLR 4.13 jar (tested with 4.13.0. and 4.13.1)
* ANTLR4 C++ runtime
* C++20 standard library
* Clang for building tests with cmake
* Boost (>=1.82)

Please see *ci/build_antlr.sh* to see how you can 
get all the ANTLR related tools. 

##Usage
Consider the following example code in file *example.cpp*:
```cpp
int main() {
    // our compiler automatically declares printf and some other functions
    printf("Hello, World!");
}
```

Once the project is build, we can call the *cpm* executable with the source code to 
be compiled:
```console
./cpm example.cpp
```
and get the LLVM IR output:
```llvm
; ModuleID = 'basic'
source_filename = "basic"

@0 = private unnamed_addr constant [14 x i8] c"Hello, World!\00", align 1

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %ret_val = alloca i32, align 4
  store i32 0, i32* %ret_val, align 4
  %0 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @0, i32 0, i32 0))
  br label %return

return:                                           ; preds = %entry
  %1 = load i32, i32* %ret_val, align 4
  ret i32 
```
We can also call the program with the 
--ast-dump option, which prints the AST:
```
TranslationUnit <line:1:1> 
|-SimpleDeclar <line:0:0>
|  -InitDeclarator <line:0:0>
|    -FunctionDecl <line:0:0> printf 'int (ptr to const char, ...)'
|      -Param <line:0:0>
|        -Decl <line:0:0> format 'ptr to const char'
| /* other implicit declarations */
 -FuncDef <line:1:1>
  |-FunctionDecl <line:1:5> main 'int ()'
   -FuncBody <line:1:12>
     -CompoundStmt <line:1:12>
       -ExprStmt <line:3:2>
         -CallExpr <line:3:2> 'int (ptr to const char, ...)', function declared on line: 0
          |-IdExpr <line:3:2> printf, declared on line 0
           -ArrToPtrExpr <line:0:0>
             -StringLiteral <line:3:9> "Hello, World!"

```

You can also run tests by calling *ctest* in the build 
directory. 
## Authors
Daniel Kral

## License
MIT license
