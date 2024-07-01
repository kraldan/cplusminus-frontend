// test that default member access inside a struct is public
struct S {
	int a;
};

int main() {
	S s;
	return s.a = 0;
}
