// test a nested if
int main() {
	int a = 42;
	if(a > 5)
		if(a > 100)
			return -1;
		else
			return a;	// this branch should be taken
	return -2;
}
