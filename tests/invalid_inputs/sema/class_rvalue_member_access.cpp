
struct S {
	int a;
}

S foo() {
	S s;
	return s;
}

int main() {
	foo().a;
}
