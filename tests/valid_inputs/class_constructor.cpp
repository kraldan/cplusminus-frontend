
struct S {
	int a;
	S(int a) {
		this->a = a;
	}
};

int main() {
	S s = S(5);
	return s.a;
}
