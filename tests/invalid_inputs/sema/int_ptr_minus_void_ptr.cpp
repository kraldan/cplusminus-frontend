// test that void pointer cannot be subtracted from int pointer

void f() {
	int *i;
	void *v;
	i - v;
}
