// see if the compiler resolves what a call refers to correctly

int global() {
	return 1;
}

// this should not be called
int implicit() {
	return -10;
}

struct S {
	int a;
	S() {
		a = 2;
	}

	int implicit() {return 4;}
	int explicit_() {return 8;}
	int foo() {
		S s = S();
		return global() + s.a + implicit() + this->explicit_();
	}
};

int main() {
	S s;
	return s.foo();
}
