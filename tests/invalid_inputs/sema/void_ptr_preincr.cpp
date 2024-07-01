// test that preincrement doesn't work with void pointers

void f() {
	void *p;
	++p;
}
