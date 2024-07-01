// test that vararg arguments have lower priority even than type conversion

int foo(char c, int i) {
	return i;
}

int foo(char c, ...) {
	return 666;
}

int main() {
	double d = 42.0;
	return foo('a', d);
}
