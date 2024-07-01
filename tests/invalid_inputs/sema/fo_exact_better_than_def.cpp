// ambiguous

int foo(int a) {
	return a;
}

int foo(int a, int b = 10) {
	return a+b;
}

int main() {
	return foo(42);
}
