// test that temporary objects are rvalues, and as such cannot be accesed, as of 02/24

struct S {
	S() {
		this->a = 5;
	}
	int a;
};

S foo() {
	return S();
}

int main() {
	return S().a + foo().a;
}
