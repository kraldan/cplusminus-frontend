// test that constructor initializer can be put after declarator name

struct S {
	int i;

	S(int n) {
		this->i = n;
	}
};

int main() {
	S s(42);
	return s.i;
}
