
struct S {
	
	int bar() { return 42; }
	int foo() { return bar(); }
}

int main() {
	S s;
	return s.foo();
}
