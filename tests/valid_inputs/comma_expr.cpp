// test a comma expression

void f() {

}

int main() {
	int a;
	// test that the expressions can have different types,
	// and that value category is preserved
	f(), 3.14, a = 42;
	return a;
}
