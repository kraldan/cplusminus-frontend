// test that the compiler catches cyclical inclusion of class within itself

struct S {
	S s;
};
