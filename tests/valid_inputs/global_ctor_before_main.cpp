// test that global constructors are called before main

int printf(const char *fmt, ...);

struct S {
	S() {
		printf("S\n");
	}
};

S s = S();

int main() {
	printf("main\n");
}
