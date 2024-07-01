// test that for loop with empty condition and return in body works correctly
// (llvm BB 'end' has no predecessors)

int main() {
	for(;;)
		return 42;
}
