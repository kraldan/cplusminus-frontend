// ambiguous call

int foo(int a) {
	return a;
}

int foo(int a, ...) {
	return -a;
}

int main() {
	return foo(42) + foo(42, 1000);
}
