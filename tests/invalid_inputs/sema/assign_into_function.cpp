
int lhs_func();
int rhs_func();

int main() {
	// make sure that rhs type matches, so that the compilation
	// truly fails on assignment into function (not 100 %, lvalue-to-rvalue on rhs would probably fail first still)
	lhs_func = rhs_func;
}
