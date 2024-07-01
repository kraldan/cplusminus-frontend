
int scanf(const char * fmt, ...);
int printf(const char * fmt, ...);

int main() {
	int a, b, ret;
	ret = scanf("%d %d", &a, &b);
    // this also tests if there's lvalue to rvalue conversion
    // on extra (vararg) arguments
	printf("%d %d\n", b, a);
    return ret;
}
