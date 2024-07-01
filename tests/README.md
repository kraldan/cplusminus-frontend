## Valid tests

Contains source code that should be compilable to llvm ir and runnable (they contain main).

Names of files try to explain what's tested. For a *basename*, there will always be:

* *basename.cpp* file with the source code

then there can exist the following files:

a) *basename.dontrun* file exists

* this file should be skipped b) *basename.ret* file exists
* this file contains the return code expected from the program
* otherwise, return code 0 is expected c) *basename.in* file exists
* its contents are sent to the program on stdin d) *basename.output* file exists
* the program output should match the contents of this file
* if this file doesn't exist, the program should have empty output on stdout e) *basename.ast* file
  exists
* this file contains the expected output of --ast-dump
  (ast dump after semantic analysis)

# Invalid tests

## Parse

Tests that should fail during parsing. Either they don't match the grammar, or the parser still
deems them wrong.

## Sema

Tests that should fail during semantic analysis.