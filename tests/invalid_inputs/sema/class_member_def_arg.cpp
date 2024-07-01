// non static members cannot be used in default argument expressions

struct S {
	int a;
	// non-static member should not be viable in a def arg value expression
	int foo(int i = a) {
		return i;
	}
};

int main() {
	S s;
     	return s.foo();
}
