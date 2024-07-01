// test that GEP is used correctly between arrays, structs and pointers

struct S {
	int a;
	int b;
}

int main() {
	S arr[5];
	S *p = &arr[0];
	p[0].a = arr[1].b = 21;
	return arr[0].a + p[1].b;
}
