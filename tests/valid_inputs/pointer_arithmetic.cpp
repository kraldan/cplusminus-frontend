// test some pointer arithmetic expressions

int main() {
	int arr[5];
	arr[3] = 42;
	return arr[&arr[1] - arr + 2];
}
