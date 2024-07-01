
int printf(const char * fmt, ...);

int foo(int i) {
	return i;
}

char foo(char c) {
	return c;
}

double foo(double d) {
	return d;
}
int main() {
	int i = foo(42);
	char c = foo('a');
	double d = foo(3.14);
	printf("%d %c %.2lf\n", i, c, d);
}
