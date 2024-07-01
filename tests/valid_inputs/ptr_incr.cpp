
int printf(char * fmt, ...);

int main() {
	// the assignment of p1, p2, p3, p4 are UB
	// (and are treated as such by -O2 pass), because
	// p is uninitialized
	int * p,
        * p1 = p,
        * p2 = p++,
        * p3 = p+1,
        * p4 = ++p;

    // if you wanted to shortcut it into:
    // 'p != p++'
    // the order of evaluation of lhs and rhs
    // is not defined. And as of the time
    // of writing this test, p++ will actually
    // be evaluated before p.

	if(p1 != p2)
        return 2;
    if(p3 != p4)
		return 3;
}
