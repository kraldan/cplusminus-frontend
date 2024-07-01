// test that instance comparison is catched during sema

struct S {

};

void f() {
	S s1, s2;
	s1 < s2;
}
