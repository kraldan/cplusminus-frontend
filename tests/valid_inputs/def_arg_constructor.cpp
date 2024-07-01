// test that default argument values work with constructors
struct S {
	int i;
	S(char c, int i = 42) {
		this->i = i;
	}
};

int main() {
	S s = S('a');
	return s.i;
}
