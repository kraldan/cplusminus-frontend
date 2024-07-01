/**
 * This program calculates the greatest common divisor of two numbers.
 *
 * It uses the simple euclidean algorithm, and prints the process.
 */


int * max(int * a, int * b) {
	return *a > *b ? a : b;
}

int * min(int * a, int * b) {
	return *a < *b ? a : b;
}

int printf(const char *fmt, ...);
int scanf(const char *fmt, ...);

int main() {
	int a, b;
	
	printf("Welcome to the gcd program.\n");
	printf("It calculates the Greatest Common Divisor of two positive integers a and b.\n");

	printf("Enter a: ");
	if ( scanf("%d", &a) != 1 || a < 1) {
		printf("This is not a valid input. Please try again.\n\n");
		return 1;
	}
	printf("Enter b: ");
	if ( scanf("%d", &b) != 1 || b < 1) {
		printf("This is not a valid input. Please try again.\n\n");
		return 2;
	}
	
	printf("Starting the euclid algorithm.\n");
	printf("a=%d b=%d\n", a, b);
	while(a != b) {
		*max(&a, &b) -= *min(&a, &b);
		printf("a=%d b=%d\n", a, b);
	}
	printf("\nGcd is: %d\n", a);
}