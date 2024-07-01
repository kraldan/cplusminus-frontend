
struct S {
        int i;

        // call constructor that has not been 
        // declared yet
        int foo(S s = S(42)) {
            return s.i;
        }
    
        S(int n) {
            i = n;
        }
};

int main() {
	S s;
	return s.foo();
}
