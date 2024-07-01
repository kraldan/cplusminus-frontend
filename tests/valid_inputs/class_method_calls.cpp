
struct A {
	int foo() {return 20;}
};

struct B {
	int foo() {return 1;}
};

int main() {
	A a;
	B b;
	return a.foo() + (&a)->foo() + b.foo() + (&b)->foo();
}
