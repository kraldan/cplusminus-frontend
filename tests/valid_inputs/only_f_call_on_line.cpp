

void foo(int * a) {
	*a = 42;
}

int main() {
	int a;
	// right now (12/23), there's a problem where foo gets parsed as a type
	foo(&a);
	return a;
}
