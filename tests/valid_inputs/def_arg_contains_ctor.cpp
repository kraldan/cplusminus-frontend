
struct S {
	int i;
	S(int n) {
		this->i = n;
	}
};

int foo(S s = S(42)) {
	return s.i;
}

int main() {
	return foo();
}
