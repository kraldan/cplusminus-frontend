// test that the compiler calls correct overload with
// const-(un)qualified pointer elements type

int foo(const int *a) {
	return *a;
}
int foo(int *a) {
	return 2**a;
}

int main() {
	int i = 14, *p = &i;
	const int *cp = &i;
	return foo(p) + foo(cp);
}
