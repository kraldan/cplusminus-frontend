// test that default arguments and vararg work with class constructor

struct S {
	int i;
	S(int n, char c = 'a', double d = 3.14, ...) {
		this->i = n;
	}
};

int main() {
	S 	s1(10, 'x', 57.0, nullptr, s1),
		s2(10, 'x', 57.0),
		s3(10, 'x'),
		s4(12);
	return s1.i + s2.i + s3.i + s4.i; 	// 42
}
