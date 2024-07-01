// test that global variable of pointer type can be declared and works

int * p;

int main() {
	int i = 42;
	p = &i;
	return *p;
}
