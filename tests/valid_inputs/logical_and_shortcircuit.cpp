
int main() {
	int *p = nullptr, i = 0;
	if(i && *p)	// *p should cause segfault
		return -1;
	return 42;
}
