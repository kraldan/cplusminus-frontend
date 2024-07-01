// test function calls for a function with multiple default argument values

int a;
int b;

int foo(int a = a, int b = b) {
	return a+b;
}

int main() {
	int sum = 0;
	a = 1, b = 10;
	sum += foo();		// 11
	sum += foo(5);		// 15
	sum += foo(2, 2);	// 4
	return sum;
}
