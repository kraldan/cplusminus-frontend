
struct S {
	int a, b;
};

int main() {
	S s;
	s.a = 5;
	(&s)->b = 6;
	return s.a + s.b;
}
