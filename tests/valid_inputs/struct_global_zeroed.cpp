// test that globally declared struct is zero initialized

struct S {
	int a;
	int b;
};

S s;

int main() {
	return s.a == 0 && s.b == 0;
}
