// test that array to pointer decay takes places with unary operator '*'

int main() {
	int arr[5];
	arr[0] = 42;
	return *arr;
}
