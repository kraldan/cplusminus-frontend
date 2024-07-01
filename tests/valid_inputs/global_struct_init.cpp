
int b;
int a = 5*b;

struct S {
	int a;
	S(int n) {
		a = n;
	}
};

S s = S(42);

int main() {
	return s.a;
}
