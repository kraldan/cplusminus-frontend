// test that post increment doesn't work with void pointers

void f() {
	void *p;
	p++;
}
