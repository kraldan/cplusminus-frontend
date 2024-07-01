// test that empty parenthesis after declarator is taken as a function
// declaration rather than call to zero arg constructor

struct S {
	int a;
	S() {
		this->a = 42;
	}
};

// declaration
S s();	// from grammar perspective, this can be both a function declaration and a constructor call

int main() {
	S s1 = s();
	return	s1.a;
}

// definition
S s() {
	return S();
}
