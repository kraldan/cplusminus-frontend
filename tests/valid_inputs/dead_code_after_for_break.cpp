int f() {
	int a;
	for(;;)
		return 42;
	// dead code, should not cause problems
	a *= 2;
}

int main() {
	return f();
}
