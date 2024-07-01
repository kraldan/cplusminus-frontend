// test that a class cannot contain itself through a 2d-array
// (we assume if 2d is checked, 1d is checked as well)

struct S {
	S arr[1][2];
};

void f() {
	// enforce LLVM IR generation
	S s;
}
