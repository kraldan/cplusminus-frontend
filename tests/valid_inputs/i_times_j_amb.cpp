// check that 'i*j' ambiguity is resolved correctly

void mul(int i, int j) {
	i*j;		// multiplication
}

struct i {
	char c;
};

void f() {
	i * j;
	j->c = 'h';
}

int main() {
	// do nothing
}
