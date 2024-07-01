
struct S {
	int a;
	int foo() {
		int res = 0;
		{ int a = 5; res += a; }
		res += a;
		return res;
	}
};

int main() {
	S s;
	s.a = 37;
	return s.foo();
}
