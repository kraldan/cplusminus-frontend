// test that void* is not implicitly convertible to int*

int main() {
	void *pv;
	int *pi = pv;	// error
}
