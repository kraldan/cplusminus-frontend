
int foo(int * p1, const int * p2) {
	return 2;
}
int foo(const int * p1, int * p2) {
	return 40;
}
int main() {
	int i;
	int * p = &i;
	const int * cp = &i;
	return foo(p, cp) + foo(cp, p);
}
