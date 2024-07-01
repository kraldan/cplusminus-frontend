
int g;

int foo(int a = g) {
	return a;
}

int main() {
	g = 10;
    // do another g_load before calling foo(),
    // see if there's name collision in the IR
    g = g+0;
	return foo();
}
