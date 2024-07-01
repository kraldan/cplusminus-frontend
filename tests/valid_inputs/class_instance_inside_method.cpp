// test that a class instance can be used inside of a class method

struct S {
	int a;
	S(int n) {
		this->a = n;
	}

	int foo() {
		S s(this->a);
		return this->a + s.a;
	}
};

int main() {
	S s(21);
	return s.foo();
}

