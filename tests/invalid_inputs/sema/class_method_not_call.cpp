
struct S {
	void f() {}
};

void foo() {
	S s;
	s.f;	// error: method used in non-call context
}
