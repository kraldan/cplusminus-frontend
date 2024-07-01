// test that a program with 3 different classes works

struct A {
	int a;
	A(int n) {a = n;}
	int f() {return a;}
};

struct B {
	char a;
	B(char n) {a = n;}
	char f() {return a;}
};

struct C {
	double a;
	C(double n) { a = n; }
	double f() {return a;}
};

int main() {
	A a(42);
	B b('x');
	C c(3.14);
	if(42 + 'x' + 3.14 == a.f() + b.f() + c.f())
		return 42;
}
