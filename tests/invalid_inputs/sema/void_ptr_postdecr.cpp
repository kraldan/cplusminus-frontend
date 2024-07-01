// test that post decrement doesn't work with void pointers

void f() {
	void *p;
	p--;
}
