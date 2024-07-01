/* Code after return has to be handled, because of llvm requiring
 * a terminator at the end of a basic block.
 *
 * bb has a method getTerminator(), that might be useful */

int main() {
    return 42;
    int d;
}