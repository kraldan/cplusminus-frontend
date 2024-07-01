// test that int* is implicitly convertible to void*
int main() {
	int *pi;
	void *pv = pi;
}
