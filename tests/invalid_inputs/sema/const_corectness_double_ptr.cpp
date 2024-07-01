// check that 'T**' is not assignable into'const T**'
// https://isocpp.org/wiki/faq/const-correctness

void f() {
	int **p1 = nullptr;
	const int **p2 = p1;	// error
}
