
struct S {
	int foo(int a) {return a;}
}

int main() {
	S s;
	return s->foo(42);
}
