// check that function cannot have the same name as a class

struct Foo {
	Foo() {}
};

int Foo() {
	return 42;
}

int main() {
	return Foo();
}
