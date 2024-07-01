
int printf(const char *fmt, ...);

int main() {
	bool t = true, f = false;
	printf("%d\n", t || t);
	printf("%d\n", t || f);
	printf("%d\n", f || t);
	printf("%d\n", f || f);
}
