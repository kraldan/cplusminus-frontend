
struct S {
	int foo(char c, int i = 21) {
//	int foo(char c, int i) {
		return i;
	}

    int foo(int i = 21) {
//    int foo(int i) {
        return i;
    }
};

int main() {
	S s;
	return s.foo('c') + s.foo();
}
