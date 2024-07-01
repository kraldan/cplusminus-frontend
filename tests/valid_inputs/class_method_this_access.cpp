
struct S {
	int a;
	int bar() {return 40;}
	int foo() {return this->a + this->bar();}
}

int main() {
	S s;
	s.a = 2;
	return s.foo();
}
