// make sure that with function definition, the correct const qual is used

int foo(const int a);

int foo(int a) {
	a = 5;  // the 'const' in the other declarations doesn't interfere here
	return 42;
}

int foo(const int a);

int main() {
	return foo(0);
}
