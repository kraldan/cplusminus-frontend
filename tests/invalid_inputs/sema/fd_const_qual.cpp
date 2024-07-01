// make sure that with function definition, the correct const qual is used

int foo(int a);

int foo(const int a) {
	a = 5;  // error
	return 42;
}

int foo(int a);
