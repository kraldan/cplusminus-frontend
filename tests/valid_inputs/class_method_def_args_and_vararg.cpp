// test that default arguments and vararg work with class non-static methods

struct S {
	// return the first argument
	int foo(int i, char c = 'a', double d = 3.14, ...) {
		return i;
	}
};

int main() {
	S s;
	return s.foo(10, 'x', 57.0, nullptr, s) + s.foo(10, 'x', 57.0) + s.foo(10, 'x') + s.foo(12); // 42
}
