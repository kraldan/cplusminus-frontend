// test that the result of a assignment is an lvalue pointing to the original value

int main() {
	int a;
	(a = 666) = 42;
	return a;
}
