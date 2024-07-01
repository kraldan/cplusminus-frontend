
# *parsing* dir
- source code that should fail during parsing
- often times things that would pass in c/c++, but shouldn't pass in this lang
- also assumptions that the ir generating compiler builds upon

# *sema* dir
- source code that should pass parsing, but fail during semantic analysis
- e.g. type checking, multiple variable declarations for one name, ...

