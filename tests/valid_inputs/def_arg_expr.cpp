
int g;

int foo(int a = g + 5) {
	return a;
}

int main() {
	g = 37;
    // add another g_load to create another temporary '%0'
    g += 0;
	return foo();
}
